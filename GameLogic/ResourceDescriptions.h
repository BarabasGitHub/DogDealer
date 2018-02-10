#pragma once

#include <Math\FloatTypes.h>

#include <Conventions\EntityTemplateID.h>
#include <Conventions\SkeletonAttachmentPoint.h>

#include <string>

namespace Logic
{
    struct MobileComponentDescription
    {
        float motion_power;
		float rotation_power;

		Math::Float2 motion_bias_positive;
		Math::Float2 motion_bias_negative;

        float target_speed;

        std::vector<AttachmentPoint> weapon_attachment_points;
    };


    struct ItemComponentDescription
    {
        EntityTemplateID equipped_template, dropped_template;
        std::string name;
    };


    struct DamageDealerComponentDescription
    {
        float damage_value;

        DamageDealerComponentDescription() = default;
        explicit DamageDealerComponentDescription(float damage_value) : damage_value(damage_value) {};
    };
}