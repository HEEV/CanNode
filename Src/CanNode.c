/*
 * CanNode.c - provides functions for using CanNode devices
 *
 * Author: Samuel Ellicott
 * Date: 6-20-16
 */

#include "CanNode.h"

static CanNode* nodes[6];

uint16_t CanNode_init(CanNode* node, CanNodeType type, CanNodePriority pri) {
	return 0;
}

bool CanNode_addFilter(uint16_t filter) {
	return true;
}

void CanNode_setFilterHandler(CanNode* node, filterHandler handle) {
}

void CanNode_checkForMessages() {
}

bool CanNode_sendMessage(CanNode* node, const CanNodeMessage* msg) {
	return true;
}


