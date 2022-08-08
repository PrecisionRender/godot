/*************************************************************************/
/*  environment_volume.cpp                                               */
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

#include "environment_volume.h"

#include "scene/3d/camera_3d.h"
#include "editor/editor_plugin.h"
#include "editor/editor_plugin.h"
#include "editor/plugins/node_3d_editor_plugin.h"
#include "scene/3d/world_environment.h"
#include "servers/rendering/storage/environment_storage.h"
#include "servers/rendering/renderer_scene_render.h"

///////////////////////////

void EnvironmentVolume::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			set_process_internal(true);
		} break;
		case NOTIFICATION_INTERNAL_PROCESS: {
			RS::get_singleton()->environment_add_blender(callable_mp(this, &EnvironmentVolume::update_environment_blending), priority, AABB(get_position() - extents, extents * 2).get_volume());
		} break;

		case NOTIFICATION_EXIT_TREE: {
			//RS::get_singleton()->environment_remove_blender(callable_mp(this, &EnvironmentVolume::update_environment_blending), priority, get_aabb().get_volume());
		} break;
	}
}

void EnvironmentVolume::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_environment", "environment"), &EnvironmentVolume::set_environment);
	ClassDB::bind_method(D_METHOD("get_environment"), &EnvironmentVolume::get_environment);

	ClassDB::bind_method(D_METHOD("set_extents", "extents"), &EnvironmentVolume::set_extents);
	ClassDB::bind_method(D_METHOD("get_extents"), &EnvironmentVolume::get_extents);

	ClassDB::bind_method(D_METHOD("set_priority", "priority"), &EnvironmentVolume::set_priority);
	ClassDB::bind_method(D_METHOD("get_priority"), &EnvironmentVolume::get_priority);

	ClassDB::bind_method(D_METHOD("set_blend_distance", "distance"), &EnvironmentVolume::set_blend_distance);
	ClassDB::bind_method(D_METHOD("get_blend_distance"), &EnvironmentVolume::get_blend_distance);

	ClassDB::bind_method(D_METHOD("set_blend_weight", "weight"), &EnvironmentVolume::set_blend_weight);
	ClassDB::bind_method(D_METHOD("get_blend_weight"), &EnvironmentVolume::get_blend_weight);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "environment", PROPERTY_HINT_RESOURCE_TYPE, "Environment"), "set_environment", "get_environment");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "extents", PROPERTY_HINT_RANGE, "0.01,1024,0.01,or_greater,suffix:m"), "set_extents", "get_extents");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "priority", PROPERTY_HINT_RANGE, "-100,100"), "set_priority", "get_priority");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "blend_distance", PROPERTY_HINT_RANGE, "0.01,1024,0.01,or_greater,exp"), "set_blend_distance", "get_blend_distance");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "blend_weight", PROPERTY_HINT_RANGE, "0,1,0.001"), "set_blend_weight", "get_blend_weight");
}

TypedArray<String> EnvironmentVolume::get_configuration_warnings() const {
	TypedArray<String> warnings = Node::get_configuration_warnings();

	Ref<Environment> environment = get_viewport()->find_world_3d()->get_environment();

	//if (environment.is_valid() && !environment->is_volumetric_fog_enabled()) {
	//	warnings.push_back(("Fog Volumes need volumetric fog to be enabled in the scene's Environment in order to be visible."));
	//}

	return warnings;
}

void EnvironmentVolume::_validate_property(PropertyInfo &property) const {
	VisualInstance3D::_validate_property(property);
}

void EnvironmentVolume::set_environment(const Ref<Environment> &p_environment) {
	if (environment == p_environment) {
		return;
	}

	environment = p_environment;
}

Ref<Environment> EnvironmentVolume::get_environment() const {
	return environment;
}

void EnvironmentVolume::set_extents(const Vector3 &p_extents) {
	extents = p_extents;
	extents.x = MAX(0.0, extents.x);
	extents.y = MAX(0.0, extents.y);
	extents.z = MAX(0.0, extents.z);
	update_gizmos();
}

Vector3 EnvironmentVolume::get_extents() const {
	return extents;
}

void EnvironmentVolume::set_priority(const int &p_priority) {
	priority = p_priority;
}

int EnvironmentVolume::get_priority() const {
	return priority;
}

void EnvironmentVolume::set_blend_distance(const real_t &p_distance) {
	blend_distance = p_distance;
}

real_t EnvironmentVolume::get_blend_distance() const {
	return blend_distance;
}

void EnvironmentVolume::set_blend_weight(const real_t &p_weight) {
	blend_weight = p_weight;
}

real_t EnvironmentVolume::get_blend_weight() const {
	return blend_weight;
}

AABB EnvironmentVolume::get_aabb() const {
	return AABB();
}

RID EnvironmentVolume::update_environment_blending(RID p_other) {
	Camera3D *camera;

	if (Engine::get_singleton()->is_editor_hint()) {
		camera = Node3DEditor::get_singleton()->get_editor_viewport(0)->get_camera_3d();
	} else {
		camera = get_viewport()->get_camera_3d();
	}

	if (camera == nullptr) {

		return p_other;
	}

	Vector3 ref_point = _get_closest_point_in_volume(AABB(get_position() - extents, extents * 2), camera->get_position());
	real_t distance = ref_point.distance_to(camera->get_position());


	real_t weight = CLAMP(((blend_distance - distance) / blend_distance) * blend_weight, 0, 1);

	if (weight < 0.5f) {
		return p_other;
	}

	//Environment other = RendererSceneRender::g::is_environment()
	//RID();
	//RS::get_singleton()->environmenr

	Ref<Environment> new_env = EnvironmentBlender::blend_environments(Ref<Environment>(environment), Ref<Environment>(p_other), weight);

	return new_env->get_rid();
}

void EnvironmentVolume::_update_current_environment() {
	WorldEnvironment *first = Object::cast_to<WorldEnvironment>(get_tree()->get_first_node_in_group("_world_environment_" + itos(get_viewport()->find_world_3d()->get_scenario().get_id())));

	if (first) {
		get_viewport()->find_world_3d()->set_environment(first->get_environment());
	} else {
		get_viewport()->find_world_3d()->set_environment(Ref<Environment>());
	}
	get_tree()->call_group_flags(SceneTree::GROUP_CALL_DEFERRED, "_world_environment_" + itos(get_viewport()->find_world_3d()->get_scenario().get_id()), "update_configuration_warnings");
}

Vector3 EnvironmentVolume::_get_closest_point_in_volume(AABB &p_aabb, Vector3 &p_point) {
	Vector3 result;
	for (int i = 0; i < 3; i++) {
		if (p_point[i] > p_aabb.get_end()[i]) {
			result[i] = p_aabb.get_end()[i];
		} else if (p_point[i] < p_aabb.get_position()[i]) {
			result[i] = p_aabb.get_position()[i];
		} else {
			result[i] = p_point[i];
		}
	}

	return result;
}

EnvironmentVolume::EnvironmentVolume() {
}

EnvironmentVolume::~EnvironmentVolume() {
	
}
