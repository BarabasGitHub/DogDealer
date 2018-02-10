#pragma once

#include "Conventions\EntityID.h"

#include <Utilities\IDGenerator.h>

typedef IDGenerator<EntityID> EntityIDGenerator;

extern template EntityIDGenerator;
