/*
 *  FeatureList.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 22.12.08.
 *  code under LGPL
 *
 */

#include "FeatureList.h"

std::set<Feature*> features;

FeatureList* FeatureList::m_instance = NULL;

void FeatureList::Init() {
	m_instance = new FeatureList();
}

FeatureList::FeatureList() {
	
}

FeatureList::~FeatureList() {

}
