#pragma once

#include <Utilities\Range.h>
#include <cstdint>

void InitializeUnionFind(Range<uint32_t *> parents);

uint32_t GetRoot(Range<uint32_t const *> parents, uint32_t index);
// gets the root and updates the parents to the grandparents in the process, this keeps the tree structure more shallow
uint32_t GetRoot(Range<uint32_t *> parents, uint32_t index);

bool Find(Range<uint32_t const*> parents, uint32_t index_a, uint32_t index_b);

void Unite(uint32_t index_a, uint32_t index_b, Range<uint32_t *> parents);