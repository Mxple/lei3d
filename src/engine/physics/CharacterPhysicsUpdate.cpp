#include "CharacterPhysicsUpdate.hpp"

#include "physics/GroundedCallback.hpp"

namespace lei3d
{
	CharacterPhysicsUpdate::CharacterPhysicsUpdate(btRigidBody* character, btCollisionObject* groundCheck, float groundCheckDist)
		: m_Character(character), m_GroundCheck(groundCheck), m_GroundCheckDist(groundCheckDist) {}

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
	void CharacterPhysicsUpdate::updateAction(btCollisionWorld* collisionWorld, btScalar deltaTime)
	{
		// Check if we are on the ground
		GroundedCallback callback(m_Character);
		collisionWorld->contactTest(m_GroundCheck, callback);
		bool onGround = callback.m_Grounded;
		bool groundPoint = callback.m_GroundPoint;
		//LEI_INFO("On Ground: {0}", onGround ? "True" : "False");

		// Update velocity accordingly
		btVector3 v = m_Character->getLinearVelocity();
		glm::vec3 prevVel = glm::vec3(v.x(), v.y(), v.z());
		// std::cout << "prevVel is: " << prevVel.x << " " << prevVel.y << " " << prevVel.z << std::endl;

		// Calculate
		glm::vec3 wishdir{ 0.0f, 0.0f, 0.0f };

		// here is where we apply our constraints during the update
		GLFWwindow* window = Application::Window();
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		{
			wishdir = wishdir + glm::vec3(1.0, 0.0, 0.0);
		}
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		{
			wishdir = wishdir + glm::vec3(-1.0, 0.0, 0.0);
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		{
			wishdir = wishdir + glm::vec3(0.0, 0.0, -1.0);
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		{
			wishdir = wishdir + glm::vec3(0.0, 0.0, 1.0);
		}

		if (wishdir != glm::vec3(0.0, 0.0, 0.0))
		{
			wishdir = glm::normalize(wishdir);
		}
		/*
		if (onGround) {
			std::cout << "on ground" << std::endl;
		} else {
			std::cout << "not on ground" << std::endl;
		}
		*/

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
			v = v + btVector3(0.0, 4.75, 0.0);
		}

		m_Character->setLinearVelocity(v);
	}

	void CharacterPhysicsUpdate::debugDraw(btIDebugDraw* debugDrawer)
	{
		//Draw Ground Check
		const btVector3 center = m_GroundCheck->getWorldTransform().getOrigin();
		const btScalar	radius = m_GroundCheckDist;
		const btVector3 groundCheckColor = btVector3(0.f, 0.f, 1.f);

		btVector3 from = { 0.f, 0.f, 0.f };
		btVector3 to = { 50.f, 0.f, 0.f };
		btVector3 color = { 1.f, 0.f, 0.f };
		debugDrawer->drawSphere(center, radius, groundCheckColor);
		//debugDrawer->drawLine(from, to, color);
		//debugDrawer->drawLine({ 0.f, 0.f, 0.f }, { 10.f, 0.f, 0.f }, { 1.f, 0.f, 0.f });
		//debugDrawer->drawSphere(center, radius, groundCheckColor); 
	}

	glm::vec3 CharacterPhysicsUpdate::Accelerate(glm::vec3 wishDir, glm::vec3 prevVel, float acceleration, float maxVelocity)
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

	glm::vec3 CharacterPhysicsUpdate::AirAcceleration(glm::vec3 wishDir, glm::vec3 prevVelocity)
	{
		return Accelerate(wishDir, prevVelocity, m_airAccel, m_maxAirSpeed);
	}

	glm::vec3 CharacterPhysicsUpdate::GroundAcceleration(glm::vec3 wishDir, glm::vec3 prevVelocity)
	{
		float speed = glm::length(prevVelocity);
		if (speed != 0)
		{
			float drop = speed * m_friction * Application::DeltaTime(); // THIS MIGHT HAVE TO BE MULTIPLIED BY DELTA TIME??
			prevVelocity *= std::max(speed - drop, 0.0f) / speed;		// Friction fall off
		}

		return Accelerate(wishDir, prevVelocity, m_accel, m_maxSpeed);
	}
} // namespace lei3d