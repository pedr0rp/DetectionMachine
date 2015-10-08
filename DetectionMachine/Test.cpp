#include "Circle.h"

int maian()
{
	Object* base = new Circle;
	Circle* der = dynamic_cast<Circle*>(base); // OK

	return 0;
}