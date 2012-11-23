//
//  PropertyTest.cpp
//  Sketchy
//
//  Created by Joshua Gargus on 4/18/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

#include <iostream>

#include "gtest/gtest.h"

#include "props/Flow.h"
#include "Eigen/Dense"


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
	
	// Add an input of the specified type.
	template<typename T>
	In<T>& 
	FlowNode::make_input() {	
		auto ptr = std::make_unique<In<T>>(*this);
		auto ret = ptr.get();
		inputs_.push_back(std::move(ptr));
		return *ret;
	}
	
	// Add an output of the specified type.
	template<typename T>
	Out<T>& 
	FlowNode::make_output() {
		auto ptr = std::make_unique<Out<T>>(*this);
		auto ret = ptr.get();
		outputs_.push_back(std::move(ptr));
		return *ret;
	}
	
	
	// XXXX
	template<typename T>
	T& vallll(In<T> &in) {
		return in.value_;
	}
	
	template<typename T>
	void setttt(Out<T> &out, T& val) {
		out.value_ = val;
	}
	
	// Allow FlowNodes to directly access the value of an In.
	int& FlowNode::val3(In<int> &in) {
		return in.value_;
	}
	template<typename T>
	T& FlowNode::val4(In<T> &in) {
		return in.value_;
	}
	template<typename T>
	T& val5(In<T> &in) {
		return in.value_;
	}
	
	// Add an output of the specified type.
	template<typename T>
	void
	FlowNode::set(Out<T> &out, T& val) {
		if (out.node_ != *this) throw std::string("cannot set output of a different node");
		out.value_ = val;
		out.valid_ = true;
	}
	
	class MatrixMultiply : public FlowNode {
	public:
		MatrixMultiply() : 
			FlowNode(),
			in1(make_input<Eigen::Matrix4f>()),
			in2(make_input<Eigen::Matrix4f>()),
			out(make_output<Eigen::Matrix4f>()) 
		{
		}
	
		In<Eigen::Matrix4f> &in1;
		In<Eigen::Matrix4f> &in2;
		Out<Eigen::Matrix4f> &out;
		
	protected:
		
		virtual void evaluate() {
			// TODO: "auto val =" causes compile error... understand why.
			Eigen::Matrix4f val = vallll(in1) * vallll(in2);
			setttt(out, val);
		}
	}; // class MatrixMultiply
	
	
	class FloatMultiply : public FlowNode {
	public:
		FloatMultiply() :
		FlowNode(),
		in1(make_input<float>()),
		in2(make_input<float>()),
		out(make_output<float>())
		{
		}
		
		In<float> &in1;
		In<float> &in2;
		Out<float> &out;
		
	protected:
		virtual void evaluate() {
			// XXXX			set(out, val(in1) * val(in2));
		}
	}; // class FloatMultiply
	
	
} // namespace Flow








TEST(FlowTest, SimpleCreation) {
	Eigen::MatrixXd m(2,2);
	m(0,0) = 3;
	m(1,0) = 2.5;
	m(0,1) = -1;
	m(1,1) = m(1,0) + m(0,1);
	
	Flow::MatrixMultiply mm1, mm2;
	Flow::FloatMultiply fm1, fm2;

//	mm1.evaluate();
	
//	mm1.in1.attach(mm2.out);
}




/*
TEST(LoopTest, SimpleCreation) {
	auto loop1 = new EventLoop();
	auto loop2 = new EventLoop();
	
	SimpleTaskPtr task1(new SimpleTask(loop1));
	SimpleTaskPtr task2(new SimpleTask(loop1));
	SimpleTaskPtr task3(new SimpleTask(loop1));
	SimpleTaskPtr task4(new SimpleTask(loop1));
	SimpleTaskPtr task5(new SimpleTask(loop1));
	
	loop1->addTask(task1);
	loop1->addTask(task2);
	loop1->addTask(task3);
	
	sleep_for(milliseconds(5));
	
	EXPECT_EQ(task1->wasRunOrAborted(), true);
	EXPECT_EQ(task2->wasRunOrAborted(), true);
	EXPECT_EQ(task3->wasRunOrAborted(), true);
	EXPECT_EQ(task4->wasRunOrAborted(), false);
	EXPECT_EQ(task1->wasCurrent(), true);
	EXPECT_EQ(task2->wasCurrent(), true);
	EXPECT_EQ(task3->wasCurrent(), true);
	EXPECT_EQ(task4->wasCurrent(), false);
	
	delete loop1;
	delete loop2;
}
*/
