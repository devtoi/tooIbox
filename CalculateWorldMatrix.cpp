#include "CalculateWorldMatrix.h"

#include <glm/glm.hpp>

void CalculateWorldMatrix( const glm::vec3& position, const glm::vec3& scale, const glm::quat& orientation, glm::mat4& world ) {
	glm::mat4x4 rotation = glm::toMat4 ( glm::normalize ( orientation ) );
	glm::vec4 newAxisX = rotation * glm::vec4 ( scale.x,	0.0f,		0.0f,		0.0f );
	glm::vec4 newAxisY = rotation * glm::vec4 ( 0.0f,		scale.y,	0.0f,		0.0f );
	glm::vec4 newAxisZ = rotation * glm::vec4 ( 0.0f,		0.0f,		scale.z,	0.0f );
	world[0][0] = newAxisX.x;	world[0][1] = newAxisX.y;	world[0][2] = newAxisX.z;	world[0][3] = 0.0f;
	world[1][0] = newAxisY.x;	world[1][1] = newAxisY.y;	world[1][2] = newAxisY.z;	world[1][3] = 0.0f;
	world[2][0] = newAxisZ.x;	world[2][1] = newAxisZ.y;	world[2][2] = newAxisZ.z;	world[2][3] = 0.0f;
	world[3][0] = position.x;	world[3][1] = position.y;	world[3][2] = position.z;	world[3][3] = 1.0f;
}

void CalculateWorldArray( const glm::vec3& position, const glm::vec3& scale, const glm::quat& orientation, float* world ) {
	glm::mat4x4 rotation = glm::toMat4 ( glm::normalize ( orientation ) );
	glm::vec4 newAxisX = rotation * glm::vec4 ( scale.x,	0.0f,		0.0f,		0.0f );
	glm::vec4 newAxisY = rotation * glm::vec4 ( 0.0f,		scale.y,	0.0f,		0.0f );
	glm::vec4 newAxisZ = rotation * glm::vec4 ( 0.0f,		0.0f,		scale.z,	0.0f );
	world[ 0] = newAxisX.x;	world[ 1] = newAxisX.y;	world[ 2] = newAxisX.z;	world[ 3] = 0.0f;
	world[ 4] = newAxisY.x;	world[ 5] = newAxisY.y;	world[ 6] = newAxisY.z;	world[ 7] = 0.0f;
	world[ 8] = newAxisZ.x;	world[ 9] = newAxisZ.y;	world[10] = newAxisZ.z;	world[11] = 0.0f;
	world[12] = position.x;	world[13] = position.y;	world[14] = position.z;	world[15] = 1.0f;
}