//
//  Flow.h
//  Sketchy
//
//  Created by Joshua Gargus on 4/12/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

#ifndef Sketchy_Propped_h
#define Sketchy_Propped_h

#include <vector>

namespace Flow {

	// Forward-declarations
	class Port;
	template<typename T> class In;
	template<typename T> class Out;
	// XXXX
	template<typename T>
	T& vallll(In<T> &in);
	
	template<typename T>
	void setttt(Out<T> &out, T& val);
	
	
	class FlowNode {
	public:
		friend class Port;
		
		FlowNode() : valid_(false), destroyed_(false) { }
		virtual ~FlowNode();

		// Invalidate self and all outputs.
		void invalidate();
		
		// Compute valid values for all outputs.
		void validate();
		
	protected:

		// Subclasses define how a flow-node computes its output values.
		// Precondition: all inputs are valid.
		// Postcondition: all outputs are valid, too.
		virtual void evaluate() = 0;

		// XXXX
		template <typename T>
		T& val2(In<T> &in) { return in.value_; }
		int& val3(In<int> &in);
		template<typename T>
		static T& val4(In<T> &in);
		
		template <typename T>
		void set(Out<T> &out, T& val);
		
		
		// Add an input of the specified type.
		template <typename T>
		In<T>& make_input();
				
		// Add an output of the specified type.
		template <typename T>
		Out<T>& make_output();
		
	private:
		bool valid_;
		bool destroyed_;			   
	
		std::vector<std::unique_ptr<Port>> inputs_;
		std::vector<std::unique_ptr<Port>> outputs_;

	}; // class FlowNode
	
	
	// XXXX
	template<typename T>
	T& val5(In<T> &in);


	class Port {
	public:
		Port(FlowNode &fn) : node_(fn), valid_(false) { }
		
		virtual void detach() = 0;

		// TODO: need to implement in In<T> and Out<T>
		virtual void invalidate() { valid_ = false; }
		
		virtual void validate() = 0;
		
	protected:
		

		bool valid_;
		FlowNode &node_;
	}; // class Port



	template<typename T>
	class PortTemplate : public Port {		
	public:
		PortTemplate(FlowNode &n) : Port(n) { }
// XXXX		const T& value() { return value_; }
		
	protected:
// XXXX		T value_;
	}; // class PortTemplate<T>


	template<typename T>
	class In : public Port {
		friend T& FlowNode::val2(In<T>&);
		friend int& FlowNode::val3(In<int>&);
		friend T& FlowNode::val4<T>(In<T>&);
		friend T& val5<T>(In<T> &in);
		template<typename TT> friend TT& vallll(In<TT>&);
		friend class Out<T>;

	public:
		In(FlowNode &fn) : Port(fn), source_(nullptr) { }
		
		virtual void detach() {
			detach(true);
		}
		
		const T& value() { return value_; }
		
	protected:
		T value_;
	
		void detach(bool ensureValidity) {
			if (source_ == nullptr) return; // already detached
			auto tmp = source_;
			source_ = nullptr;
			tmp->detach(*this);
			
			// If we haven't lazily-evaluated our value, do so now.
			// Otherwise the values computed by the dataflow will
			// depend on when various outputs are observed... crazy!!
			// Just because we're using lazy evaluation doesn't mean that
			// there isn't a single well-defined value at any point in time.
			if (!Port::valid_) {
				// However, evaluating the source's value can be expensive...
				// don't do it if we know that it doesn't matter (for example
				// when we're replacing the source with a different one).
				if (ensureValidity) value_ = tmp->value();
			}
		}
		
		// TODO: use & instead of * (for both args and member vars)
		void attach(Out<T> &src) {
			
			/// Check if already attached to this source.
			if (&src == source_) return;

			// Detach ourself from current source, if any.
			detach(false);
			
			// Attach ourself to new source.
			source_ = &src;
			source_->attach(this);
			
			// Check if we need to invalidate our flow-node.  If we are already
			// invalid, no need to invalidate again.  If we are valid and the
			// new source's value matches ours, no need to invalidate.
			if (!Port::valid_) return;
			if ( source_->valid_ && (PortTemplate<T>::value() == source_->value()) ) return;
			invalidate();
		}

		virtual void invalidate() {
			Port::node_.invalidate();
		}
		
		virtual void validate() {
			Port::node_.validate();
		}
		

	protected:
		Out<T> *source_;

	}; // class In<T>



	template<typename T>
	class Out : public PortTemplate<T> {
		template<typename TT> friend void setttt(Out<TT>&, TT&);
		friend class In<T>; 
		
	public:
		Out(FlowNode &fn) : PortTemplate<T>(fn) { }
		
		const T& value() {
			if (!Port::valid_) Port::node_.validate();
			// XXXX assert(Port::valid_);
			return value_;
		}
		
		// Detach all consumers.
		virtual void detach() {
			int count = consumers_.size();
			while (count > 0) {
				consumers_.front()->detach();
				if (--count != consumers_.size()) {
					// We double-dispatch to the consumer
					// to let it detach itself; if we do
					// this and the consumer-list doesn't
					// shrink, we have a problem.
					throw std::string("consumer not properly removed");
				}
			}
			
		}
		
		void detach(In<T> &in) {
			auto it = std::find(consumers_.begin(), consumers_.end(), &in);
			if (it != consumers_.end()) {
				consumers_.erase(it);
				
				// TODO: uugh, this will cause a second traversal of consumers_, at least if
				// we're invoked first.  However, typically the consumer will be called with
				// detach, so when we call it back it won't call us again.  So maybe this method
				// (detach-with-consumer-as-arg) should be hidden from app-developers?
				in.detach();
			}
		}
	protected:
		T value_;
		
		virtual void validate() {
			// TODO: what should validate do?
			throw std::string("hmm... what should Out.validate() do?");
		}
		
		
		// TODO: figure out why I have trouble when trying references instead of ptrs.
		std::vector<In<T>*> consumers_;
		
		
	}; // class Out<T>

	
	
	
	// XXXX
	// Allow FlowNodes to directly access the value of an In.
//	template<typename T>
//	T& FlowNode::val2(In<T> &in) {
//		return in.value_;
//	}
	
} // namespace Flow

#endif
