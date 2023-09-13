#include "components/CharacterController.hpp"

#include "core/Application.hpp"

#include "physics/GroundedCallback.hpp"

#include <algorithm>

#include <GLFW/glfw3.h>

namespace lei3d
{
	CharacterController::CharacterPhysicsUpdate::CharacterPhysicsUpdate(CharacterController& controller, btRigidBody* character, btCollisionObject* groundCheck, float groundCheckDist)
		: m_Controller(controller), m_Character(character), m_GroundCheck(groundCheck), m_GroundCheckDist(groundCheckDist) {}

	glm::vec3 projectVector(glm::vec3& vecToProject, glm::vec3& targetVector)
	{
		float dotProduct = glm::dot(vecToProject, targetVector);
		float targetLengthSquared = glm::length(targetVector) * glm::length(targetVector);
		return targetVector * (dotProduct / targetLengthSquared);
	}

	/**
	 * CharacterPhysicsUpdate -> this is the airAccelerate function that allows airstrafing to work.
	 * wishdir is the direction that the player presses on the keyboard to indicate the direction they would like to move.
	 *
	 * @param  {btCollisionWorld*} collisionWorld :
	 * @param  {btScalar} deltaTime               :
	 */
	void CharacterController::CharacterPhysicsUpdate::updateAction(btCollisionWorld* collisionWorld, btScalar deltaTime)
	{
		// Check if we are on the ground
		GroundedCallback callback(m_Character);
		collisionWorld->contactTest(m_GroundCheck, callback);

		bool onGround = callback.m_Grounded;
		if (m_Controller.m_IncludeSFX && m_Controller.m_Grounded == false && onGround == true)
		{
			AudioPlayer::PlaySFX("landing_2"); //.PlaySound("landing");
		}
		bool groundPoint = callback.m_GroundPoint;
		m_Controller.m_Grounded = onGround;

		// Update velocity accordingly
		btVector3 v = m_Character->getLinearVelocity();
		glm::vec3 prevVel = glm::vec3(v.x(), v.y(), v.z());
		// std::cout << "prevVel is: " << prevVel.x << " " << prevVel.y << " " << prevVel.z << std::endl;

		// Calculate
		glm::vec3 wishdir{ 0.0f, 0.0f, 0.0f };
		float	  yawRotationRadian = glm::radians(m_Controller.m_Entity.m_Transform.yawRotation);

		// here is where we apply our constraints during the update
		GLFWwindow* window = Application::Window();
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			//Same as Camera
			glm::vec3 forwardVec = glm::normalize(glm::vec3(cos(yawRotationRadian), 0, sin(yawRotationRadian)));
			wishdir = wishdir + forwardVec;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			//Same as Camera
			glm::vec3 forwardVec = glm::normalize(glm::vec3(cos(yawRotationRadian), 0, sin(yawRotationRadian)));
			wishdir = wishdir - forwardVec;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			//Same as Camera
			glm::vec3 rightVec = glm::normalize(glm::vec3(-sin(yawRotationRadian), 0, cos(yawRotationRadian)));
			wishdir = wishdir - rightVec;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			//Same as Camera
			glm::vec3 rightVec = glm::normalize(glm::vec3(-sin(yawRotationRadian), 0, cos(yawRotationRadian)));
			wishdir = wishdir + rightVec;
		}

		if (wishdir != glm::vec3(0.0, 0.0, 0.0))
		{
			wishdir = glm::normalize(wishdir);
		}

		if (onGround)
		{
			glm::vec3 outputVel = GroundAcceleration(wishdir, prevVel);
			v = btVector3(outputVel.x, outputVel.y, outputVel.z);
		}
		else
		{
			glm::vec3 outputVel = AirAcceleration(wishdir, prevVel);
			v = btVector3(outputVel.x, outputVel.y, outputVel.z);
		}

		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && onGround)
		{
			v = v + btVector3(0, m_Controller.m_jumpPower, 0);
		}

		m_Character->setLinearVelocity(v);
	}

	void CharacterController::CharacterPhysicsUpdate::debugDraw(btIDebugDraw* debugDrawer)
	{
		// Draw Ground Check
		const btVector3 center = m_GroundCheck->getWorldTransform().getOrigin();
		const btScalar	radius = m_GroundCheckDist;
		const btVector3 groundCheckColor = btVector3(0.f, 0.f, 1.f);

		debugDrawer->drawSphere(center, radius, groundCheckColor);
	}

	glm::vec3 CharacterController::CharacterPhysicsUpdate::Accelerate(glm::vec3 wishDir, glm::vec3 prevVel, float acceleration, float maxVelocity)
	{
		float projectedSpeed = glm::dot(prevVel, wishDir);
		float wishSpeed = acceleration * Application::DeltaTime(); // this is the wish speed (MIGHT NEED DELTA TIME TO FIX THIS????

		// If necessary, truncate the new speed so it doesn't exceed max velocity
		if (projectedSpeed + wishSpeed > maxVelocity)
		{
			wishSpeed = maxVelocity - projectedSpeed;
		}

		return prevVel + wishDir * wishSpeed;
	}

	glm::vec3 CharacterController::CharacterPhysicsUpdate::AirAcceleration(glm::vec3 wishDir, glm::vec3 prevVelocity)
	{
		return Accelerate(wishDir, prevVelocity, m_Controller.m_airAccel, m_Controller.m_maxAirSpeed);
	}

	glm::vec3 CharacterController::CharacterPhysicsUpdate::GroundAcceleration(glm::vec3 wishDir, glm::vec3 prevVelocity)
	{
		float speed = glm::length(prevVelocity);
		if (speed != 0)
		{
			float drop = speed * m_Controller.m_friction * Application::DeltaTime(); // THIS MIGHT HAVE TO BE MULTIPLIED BY DELTA TIME??
			prevVelocity *= std::max(speed - drop, 0.0f) / speed;					 // Friction fall off
		}

		return Accelerate(wishDir, prevVelocity, m_Controller.m_accel, m_Controller.m_maxSpeed);
	}
} // namespace lei3d