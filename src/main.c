#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

typedef struct { 
    double x, y, z; 
} Vector3;
typedef struct { 
    Vector3 a, b, c; 
} Triangle;
typedef struct { 
    Vector3 min, max; 
} BoundingBox;

Triangle *segitiga = NULL;
size_t tri_count = 0;
size_t tri_cap = 0;

BoundingBox *vox = NULL;
size_t vox_count = 0;
size_t vox_cap = 0;

int depth_count[30] = {0};
int pruned_count[30] = {0};

Vector3 vec_sub(Vector3 a, Vector3 b) { 
    return (Vector3){a.x - b.x, a.y - b.y, a.z - b.z}; 
}
Vector3 vec_cross(Vector3 a, Vector3 b) { 
    return (Vector3){a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x}; 
}
double vec_dot(Vector3 a, Vector3 b) { 
    return a.x*b.x + a.y*b.y + a.z*b.z; 
}

void add_voxel(BoundingBox v) {
    if (vox_count >= vox_cap) {
        if (vox_cap == 0) {
            vox_cap = 1024;
        } else {
            vox_cap = vox_cap * 2;
        }
        vox = realloc(vox, vox_cap * sizeof(BoundingBox));
    }
    vox[vox_count++] = v;
}

void project_box(BoundingBox b, Vector3 axis, double *min_out, double *max_out) {
    Vector3 corners[8] = {
        {b.min.x, b.min.y, b.min.z}, {b.max.x, b.min.y, b.min.z},
        {b.min.x, b.max.y, b.min.z}, {b.max.x, b.max.y, b.min.z},
        {b.min.x, b.min.y, b.max.z}, {b.max.x, b.min.y, b.max.z},
        {b.min.x, b.max.y, b.max.z}, {b.max.x, b.max.y, b.max.z}
    };
    *min_out = *max_out = vec_dot(corners[0], axis);
    for (int i = 1; i < 8; i++) {
        double p = vec_dot(corners[i], axis);
        if (p < *min_out) *min_out = p;
        if (p > *max_out) *max_out = p;
    }
}

bool check_intersection(int tri_idx, BoundingBox box) {
    Triangle *t = &segitiga[tri_idx];

    double min_x = fmin(t->a.x, fmin(t->b.x, t->c.x));
    double max_x = fmax(t->a.x, fmax(t->b.x, t->c.x));
    double min_y = fmin(t->a.y, fmin(t->b.y, t->c.y));
    double max_y = fmax(t->a.y, fmax(t->b.y, t->c.y));
    double min_z = fmin(t->a.z, fmin(t->b.z, t->c.z));
    double max_z = fmax(t->a.z, fmax(t->b.z, t->c.z));

    if (min_x > box.max.x || max_x < box.min.x) return false;
    if (min_y > box.max.y || max_y < box.min.y) return false;
    if (min_z > box.max.z || max_z < box.min.z) return false;

    Vector3 edge1 = vec_sub(t->b, t->a);
    Vector3 edge2 = vec_sub(t->c, t->a);
    Vector3 normal = vec_cross(edge1, edge2);

    double d = vec_dot(normal, t->a);
    Vector3 extents = {(box.max.x - box.min.x) / 2.0, (box.max.y - box.min.y) / 2.0, (box.max.z - box.min.z) / 2.0};
    Vector3 center = {(box.max.x + box.min.x) / 2.0, (box.max.y + box.min.y) / 2.0, (box.max.z + box.min.z) / 2.0};

    double r = fabs(normal.x) * extents.x + fabs(normal.y) * extents.y + fabs(normal.z) * extents.z;
    double s = vec_dot(normal, center) - d;
    if (fabs(s) > r) return false;

    Vector3 edges[3] = { 
        vec_sub(t->b, t->a), 
        vec_sub(t->c, t->b), 
        vec_sub(t->a, t->c) 
    };

    Vector3 box_axes[3] = { 
        {1,0,0}, 
        {0,1,0}, 
        {0,0,1} 
    };

    for (int ei = 0; ei < 3; ei++) {
        for (int ai = 0; ai < 3; ai++) {
            Vector3 axis = vec_cross(edges[ei], box_axes[ai]);
            if (vec_dot(axis, axis) >= 1e-12) {
                double p1 = vec_dot(t->a, axis);
                double p2 = vec_dot(t->b, axis);
                double p3 = vec_dot(t->c, axis);
                
                double tmin = fmin(p1, fmin(p2, p3));
                double tmax = fmax(p1, fmax(p2, p3));

                double bmin, bmax;
                project_box(box, axis, &bmin, &bmax);

                if (tmax < bmin || bmax < tmin) return false;
            }
        }
    }
    return true;
}

void process_space(BoundingBox box, int current_level, int max_level, int *active_segitiga, int active_count) {
    depth_count[current_level]++;

    if (active_count == 0) {
        pruned_count[current_level]++;
        return;
    }

    if (current_level == max_level) {
        add_voxel(box);
        return;
    }

    Vector3 center = {
        (box.min.x + box.max.x) / 2.0,
        (box.min.y + box.max.y) / 2.0,
        (box.min.z + box.max.z) / 2.0
    };

    BoundingBox children[8];
    for (int i = 0; i < 8; i++) {
        children[i].min.x = (i & 1) ? center.x : box.min.x;
        children[i].max.x = (i & 1) ? box.max.x : center.x;
        children[i].min.y = (i & 2) ? center.y : box.min.y;
        children[i].max.y = (i & 2) ? box.max.y : center.y;
        children[i].min.z = (i & 4) ? center.z : box.min.z;
        children[i].max.z = (i & 4) ? box.max.z : center.z;

        int *buffer = malloc(active_count * sizeof(int));
        int pass_count = 0;

        for (int j = 0; j < active_count; j++) {
            if (check_intersection(active_segitiga[j], children[i])) {
                buffer[pass_count++] = active_segitiga[j];
            }
        }
        process_space(children[i], current_level + 1, max_level, buffer, pass_count);
        free(buffer);
    }
}

bool read_obj(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return false;

    Vector3 *points = NULL;
    size_t num_points = 0, capacity_points = 0;
    char line[512];

    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "v ", 2) == 0) {
            Vector3 p;
            if (sscanf(line + 2, "%lf %lf %lf", &p.x, &p.y, &p.z) == 3) {
                if (num_points >= capacity_points) {
                    capacity_points = capacity_points == 0 ? 1024 : capacity_points * 2;
                    points = realloc(points, capacity_points * sizeof(Vector3));
                }
                points[num_points++] = p;
            }
        } else if (strncmp(line, "f ", 2) == 0) {
            int v[10]; 
            int parsed = 0;
            char *token = strtok(line + 2, " \r\n");
            
            while (token && parsed < 10) {
                if (sscanf(token, "%d", &v[parsed]) == 1) {
                    parsed++;
                }
                token = strtok(NULL, " \r\n");
            }

            for (int i = 1; i < parsed - 1; i++) {
                if (tri_count >= tri_cap) {
                    tri_cap = tri_cap == 0 ? 1024 : tri_cap * 2;
                    segitiga = realloc(segitiga, tri_cap * sizeof(Triangle));
                }
                segitiga[tri_count++] = (Triangle){points[v[0]-1], points[v[i]-1], points[v[i+1]-1]};
            }
        }
    }
    free(points);
    fclose(f);
    return tri_count > 0;
}

void write_obj(const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) return;

    double ox[8] = {0,1,1,0, 0,1,1,0}, oy[8] = {0,0,1,1, 0,0,1,1}, oz[8] = {0,0,0,0, 1,1,1,1};
    int faces[6][4] = {{0,3,2,1}, {4,5,6,7}, {0,1,5,4}, {3,7,6,2}, {0,4,7,3}, {1,2,6,5}};

    for (size_t a = 0; a < vox_count; a++) {
        BoundingBox k = vox[a];
        Vector3 size = vec_sub(k.max, k.min);

        for (int i = 0; i < 8; i++) {
            fprintf(f, "v %.6f %.6f %.6f\n", k.min.x + ox[i]*size.x, k.min.y + oy[i]*size.y, k.min.z + oz[i]*size.z);
        }
        
        long base = (long)a * 8 + 1;
        for (int j = 0; j < 6; j++) {
            fprintf(f, "f %ld %ld %ld %ld\n", base+faces[j][0], base+faces[j][1], base+faces[j][2], base+faces[j][3]);
        }
    }
    fclose(f);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Cara penggunaan: %s <input.obj> <kedalaman>\n", argv[0]);
        return 1;
    }

    const char *input_path = argv[1];
    int max_depth = atoi(argv[2]);
    
    if (max_depth < 0 || max_depth > 20) {
        printf("Kedalaman harus berada di antara 0 dan 20.\n");
        return 1;
    }

    char output_path[512];
    snprintf(output_path, sizeof(output_path), "%s_hasil.obj", input_path);

    FILE *test = fopen(input_path, "r");
    if (!test) {
        printf("File '%s' tidak ditemukan.\n", input_path);
        return 1;
    }
    fclose(test);

    if (!read_obj(input_path)) {
        printf("File tidak valid\n");
        return 1;
    }

    BoundingBox root_box = {segitiga[0].a, segitiga[0].a};
    for (size_t i = 0; i < tri_count; i++) {
        Vector3 pts[3] = {segitiga[i].a, segitiga[i].b, segitiga[i].c};
        for(int j=0; j<3; j++) {
            root_box.min.x = fmin(root_box.min.x, pts[j].x);
            root_box.min.y = fmin(root_box.min.y, pts[j].y);
            root_box.min.z = fmin(root_box.min.z, pts[j].z);
            root_box.max.x = fmax(root_box.max.x, pts[j].x);
            root_box.max.y = fmax(root_box.max.y, pts[j].y);
            root_box.max.z = fmax(root_box.max.z, pts[j].z);
        }
    }

    Vector3 diff = vec_sub(root_box.max, root_box.min);
    double max_extent = fmax(diff.x, fmax(diff.y, diff.z));
    double pad = max_extent * 0.001;
    
    root_box.min.x -= pad; root_box.min.y -= pad; root_box.min.z -= pad;
    root_box.max.x = root_box.min.x + max_extent + pad * 2.0;
    root_box.max.y = root_box.min.y + max_extent + pad * 2.0;
    root_box.max.z = root_box.min.z + max_extent + pad * 2.0;

    int *all_tris = malloc(tri_count * sizeof(int));
    for (size_t i = 0; i < tri_count; i++) all_tris[i] = i;

    clock_t start = clock();
    process_space(root_box, 0, max_depth, all_tris, tri_count);
    clock_t end = clock();
    
    free(all_tris);
    write_obj(output_path);

    printf("\nBanyaknya voxel yang terbentuk: %zu\n", vox_count);
    printf("Banyaknya vertex yang terbentuk: %lu\n", (unsigned long)vox_count * 8);
    printf("Banyaknya faces yang terbentuk: %lu\n\n", (unsigned long)vox_count * 6);

    printf("Statistik node octree yang terbentuk:\n");
    for (int i = 1; i <= max_depth; i++) {
        printf("%d : %d\n", i, depth_count[i]);
    }

    printf("\nStatistik node yang tidak perlu ditelusuri:\n");
    for (int i = 1; i <= max_depth; i++) {
        printf("%d : %d\n", i, pruned_count[i]);
    }

    printf("\nKedalaman octree: %d\n", max_depth);
    printf("Lama waktu program berjalan: %.3f s\n", (double)(end - start) / CLOCKS_PER_SEC);
    printf("Path file .obj disimpan: %s\n\n", output_path);

    free(segitiga);
    free(vox);
    return 0;
}