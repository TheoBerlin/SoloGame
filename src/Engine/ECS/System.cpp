#include "System.hpp"

void System::RegisterSystem(const std::string& systemName, SystemRegistration& systemRegistration)
{
    m_SystemName = systemName;

    const RegularWorkInfo regularWorkInfo = {
        .TickFunction = std::bind_front(&System::Update, this),
        .EntitySubscriberRegistration = systemRegistration.SubscriberRegistration,
        .Phase = systemRegistration.Phase,
        .TickPeriod = systemRegistration.TickFrequency == 0 ? 0.0f : 1.0f / systemRegistration.TickFrequency
    };

    SubscribeToEntities(systemRegistration.SubscriberRegistration);
    ScheduleRegularWork(regularWorkInfo);
}
