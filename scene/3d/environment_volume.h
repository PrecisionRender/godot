/*************************************************************************/
/*  environment_volume.h                                                 */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef ENVIRONMENT_VOLUME_H
#define ENVIRONMENT_VOLUME_H

#include "core/templates/rid.h"
#include "scene/3d/visual_instance_3d.h"
#include "scene/main/node.h"
#include "scene/main/viewport.h"
#include "scene/resources/material.h"

class EnvironmentVolume : public VisualInstance3D {
	GDCLASS(EnvironmentVolume, VisualInstance3D);

	Ref<Environment> environment;

	Vector3 extents = Vector3(1, 1, 1);

	int priority = 0;
	real_t blend_distance = 0.5;
	real_t blend_weight = 1;

	RID volume;

	void _update_current_environment();
	Vector3 _get_closest_point_in_volume(AABB &p_aabb, Vector3 &p_point);

protected:
	_FORCE_INLINE_ RID _get_volume() { return volume; }
	void _notification(int p_what);
	static void _bind_methods();
	virtual void _validate_property(PropertyInfo &property) const override;

public:
	RID update_environment_blending(RID p_other);

	void set_environment(const Ref<Environment> &p_environment);
	Ref<Environment> get_environment() const;

	void set_extents(const Vector3 &p_extents);
	Vector3 get_extents() const;

	void set_priority(const int &p_priority);
	int get_priority() const;

	void set_blend_distance(const real_t &p_distance);
	real_t get_blend_distance() const;

	void set_blend_weight(const real_t &p_weight);
	real_t get_blend_weight() const;

	virtual AABB get_aabb() const override;
	TypedArray<String> get_configuration_warnings() const override;

	EnvironmentVolume();
	~EnvironmentVolume();
};

#endif // ENVIRONMENT_VOLUME_H
