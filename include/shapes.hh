#ifndef SHAPES_H
#define SHAPES_H

#include <raylib.h>

typedef enum {
	ShapeTypeSphere,
	ShapeTypeCapsule,
	ShapeTypeCuboid,
	ShapeTypeCube,
} ShapeType;

typedef struct {
	float radius;
} ShapeSphere;

typedef struct { /* TODO */ } ShapeCapsule;
typedef struct { /* TODO */ } ShapeCuboid;
typedef struct { /* TODO */ } ShapeCube;

typedef struct {
	union {
		ShapeSphere  sphere;
		ShapeCapsule capsule;
		ShapeCuboid  cuboid;
		ShapeCube    cube;
	} shape;

	ShapeType type;
	Vector3   pos;
	Color     col;
} Target;


#endif /* SHAPES_H */

