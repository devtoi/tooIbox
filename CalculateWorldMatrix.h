#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/quaternion.hpp>
#include "UtilityLibraryDefine.h"

UTILITY_API void CalculateWorldMatrix( const glm::vec3& position, const glm::vec3& scale, const glm::quat& orientation, glm::mat4& world );
UTILITY_API void CalculateWorldArray( const glm::vec3& position, const glm::vec3& scale, const glm::quat& orientation, float* world );