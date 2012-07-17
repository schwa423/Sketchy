//
//  Flow.cpp
//  Sketchy
//
//  Created by Joshua Gargus on 4/22/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

#include <iostream>

#include "props/Flow.h"


namespace std {

	// Herb Sutter's implementation of make_unique();
	// TODO: put this in a suitable header.
	template<typename T, typename ...Args>
	std::unique_ptr<T> make_unique( Args&& ...args )
	{
		return std::unique_ptr<T>( new T( std::forward<Args>(args)... ) );
	}

} // namespace std

namespace Flow {
	
	FlowNode::~FlowNode() {
		destroyed_ = true;
		for (auto &in : inputs_) in->detach();
		for (auto &out : outputs_) out->detach();
	}
	
	// Invalidate self and all outputs.
	void FlowNode::invalidate() {
		if (valid_) {
			valid_ = false;
			for (auto &out : outputs_) out->invalidate();
		}
	}
	
	// Compute valid values for all outputs.
	void FlowNode::validate() {
		if (valid_) return;  // already valid, nothing to do.
		if (destroyed_) throw std::string("cannot evaluate destroyed flow-node");
		for (auto &in : inputs_) in->validate();
		evaluate();
		valid_ = true;
	}
	
	// Add an input of the specified type.
	template<typename T>
	In<T>& 
	FlowNode::make_input() {
		auto ptr = std::make_unique<In<T>>();
		auto ret = ptr.get();
		inputs_.push_back(std::move(ptr));
		return *ret;
	}
	
	// Add an output of the specified type.
	template<typename T>
	Out<T>& 
	FlowNode::make_output() {
		auto ptr = std::make_unique<Out<T>>();
		auto ret = ptr.get();
		outputs_.push_back(std::move(ptr));
		return *ret;
	}
	
} // namespace Flow