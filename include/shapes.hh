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

typedef struct {
	float x;
	float y;
	float z;
	float width;
	float height;
	float length;
} ShapeCuboid;

typedef struct {
	float x;
	float y;
	float z;
	float width;
	float height;
} ShapeCube;

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

	float     time_ms;
	bool      visible;
	bool      hit;
} Target;

typedef struct {
	Vector3 start;
	Vector3 end;
} Line;

#endif /* SHAPES_H */

