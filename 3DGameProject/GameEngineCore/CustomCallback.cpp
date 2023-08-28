#include "PrecompileHeader.h"
#include "CustomCallback.h"

void CustomCallback::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
{
	while (nbPairs--)
	{
		physx::PxContactPair current = *pairs++;
		// 액터가 가지고 있는 쉐이프를 모두 가져옴
		physx::PxShape* tmpContactActor = current.shapes[0];
		physx::PxShape* tmpOtherActor = current.shapes[1];
		physx::PxFilterData OtherFilterdata = tmpOtherActor->getSimulationFilterData();
		physx::PxFilterData ContactFilterdata = tmpContactActor->getSimulationFilterData();
		if (OtherFilterdata.word0 & static_cast<physx::PxU32>(PhysXFilterGroup::Ground) &&
			ContactFilterdata.word0 & static_cast<physx::PxU32>(PhysXFilterGroup::PlayerDynamic))
		{

			if (current.events & physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
			{
				int a = 0;
				//Component->IsGround_true();
			}
			if (current.events & physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
			{
				int a = 0;
				//Component->IsGround_false();
			}

		}
		if (OtherFilterdata.word0 & static_cast<physx::PxU32>(PhysXFilterGroup::PlayerDynamic) &&
			ContactFilterdata.word0 & static_cast<physx::PxU32>(PhysXFilterGroup::Obstacle) &&
			current.events & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
		{
			int a = 0;
		}

		if (OtherFilterdata.word0 & static_cast<physx::PxU32>(PhysXFilterGroup::Obstacle) &&
			ContactFilterdata.word0 & static_cast<physx::PxU32>(PhysXFilterGroup::PlayerDynamic) &&
			current.events & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
		{
			int a = 0;
		}
	}
}

void CustomCallback::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
{
	while (count--)
	{
		physx::PxTriggerPair& current = *pairs++;

		// 액터가 가지고 있는 쉐이프를 모두 가져옴
		physx::PxRigidActor& tmpOtherActor = *current.otherActor;
		physx::PxFilterData OtherFilterdata = current.otherShape->getSimulationFilterData();
		physx::PxFilterData TriggerFilterdata = current.triggerShape->getSimulationFilterData();
		physx::PxU16 tmpnbShape = current.otherActor->getNbShapes();

		//C26813  : 비트플래그로 사용된 enum끼리의 비교는 == 이 아닌 bitwise and(&)로 비교하는 것이 좋음
		//WARNING : resultFd.word0 == static_cast<physx::PxU32>(PhysXFilterGroup::Ground
		//충돌체의 filterData가 ground이면서 닿았을 경우

		if (OtherFilterdata.word0 & static_cast<physx::PxU32>(PhysXFilterGroup::Ground) &&
			TriggerFilterdata.word0 & static_cast<physx::PxU32>(PhysXFilterGroup::PlayerFace) &&
			current.status & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
		{

			//if (Component != nullptr)
			//{
			//	return;
			//}
		}
	}
}