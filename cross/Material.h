//
//  Material.h
//  Sketchy
//
//  Created by Joshua Gargus on 2/12/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

// Immutable, so we don't need a shadow.

#ifndef Sketchy_Material_h
#define Sketchy_Material_h


namespace Sketchy {

	class Material
	{
	public:
		Material(GLfloat color[4], );

	private:
		GLfloat m_color[4];
	}; // class Material

} // namespace Sketchy

#endif
