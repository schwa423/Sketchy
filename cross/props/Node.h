//
//  Node.h
//  Sketchy
//
//  Created by Joshua Gargus on 4/12/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

#ifndef Sketchy_Propped_h
#define Sketchy_Propped_h

namespace Prop {

	class Node {
		friend class Port;
	public:


	protected:
		virtual void invalidate(Node* n = nullptr) = 0;

	}; // class Node



	class Port {
	public:
		Port(Node &n) : node_(n), valid_(false) { }

	protected:
		virtual void invalidate() { valid_ = false; }

		bool valid_;
		Node const& node_;
	}; // class Port



	template<typename T>
	class Port<T> : public Port {

	protected:
		T value_;
	}; // class Port<T>



	// Forward-declarations
	template<typename T> class In<T>;
	template<typename T> class Out<T>;



	template<typename T>
	class In<T> : public Port<T>, source_(nullptr) {
		friend class Out<T>;

	protected:

		// TODO: use & instead of * (for both args and member vars)
		void attach(Out *src) {

			/// Check if already attached to this source.
			if (src == source_) return;

			// Detach ourself from current source.
			if (source_ != nullptr) {
				auto tmp = source_;
				source_ = nullptr;
				source_.detach(this);
			}

			// Attach ourself to new source.
			source_ = src;
			if (source_ != nullptr)	source_.attach(this);

			// Check if we need to invalidate our Node.  If we are already
			// invalid, no need to invalidate again.  If we are valid and the
			// new source's value matches ours, no need to invalidate.
			if (!valid_) return;
			if (source_.valid_ && (value_ == source_.value_)) return;
			invalidate();
		}

		virtual void invalidate() {
			node_.invalidate(this);
		}

	protected:
		Out<T> *source_;

	}; // class In<T>



	template<typename T>
	class Out<T> : public Port<T> {

	protected:
		std::vector<In<T>*> consumers_;

	}; // class Out<T>

} // namespace Props

#endif
