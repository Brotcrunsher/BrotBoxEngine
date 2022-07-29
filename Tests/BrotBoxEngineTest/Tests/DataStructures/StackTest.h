#pragma once

#include "BBE/Stack.h"
#include "BBE/UtilTest.h"

namespace bbe
{
	namespace test
	{
		void testStack()
		{
			{
				Stack<int> stack;
				assertEquals(stack.dataLeft(), 0);
				assertEquals(stack.hasDataLeft(), false);

				stack.push(0);
				assertEquals(stack.dataLeft(), 1);
				assertEquals(stack.hasDataLeft(), true);
				
				assertEquals(stack.peek(), 0);
				assertEquals(stack.dataLeft(), 1);
				assertEquals(stack.hasDataLeft(), true);

				assertEquals(stack.pop(), 0);
				assertEquals(stack.dataLeft(), 0);
				assertEquals(stack.hasDataLeft(), false);

				stack.push(1);
				assertEquals(stack.dataLeft(), 1);
				assertEquals(stack.hasDataLeft(), true);

				assertEquals(stack.peek(), 1);
				assertEquals(stack.dataLeft(), 1);
				assertEquals(stack.hasDataLeft(), true);

				stack.push(2);
				assertEquals(stack.dataLeft(), 2);
				assertEquals(stack.hasDataLeft(), true);

				assertEquals(stack.peek(), 2);
				assertEquals(stack.dataLeft(), 2);
				assertEquals(stack.hasDataLeft(), true);

				stack.push(3);
				assertEquals(stack.dataLeft(), 3);
				assertEquals(stack.hasDataLeft(), true);

				assertEquals(stack.peek(), 3);
				assertEquals(stack.dataLeft(), 3);
				assertEquals(stack.hasDataLeft(), true);

				stack.push(4);
				assertEquals(stack.dataLeft(), 4);
				assertEquals(stack.hasDataLeft(), true);

				assertEquals(stack.peek(), 4);
				assertEquals(stack.dataLeft(), 4);
				assertEquals(stack.hasDataLeft(), true);

				stack.push(5);
				assertEquals(stack.dataLeft(), 5);
				assertEquals(stack.hasDataLeft(), true);

				assertEquals(stack.peek(), 5);
				assertEquals(stack.dataLeft(), 5);
				assertEquals(stack.hasDataLeft(), true);

				assertEquals(stack.pop(), 5);
				assertEquals(stack.dataLeft(), 4);
				assertEquals(stack.hasDataLeft(), true);

				assertEquals(stack.pop(), 4);
				assertEquals(stack.dataLeft(), 3);
				assertEquals(stack.hasDataLeft(), true);

				assertEquals(stack.pop(), 3);
				assertEquals(stack.dataLeft(), 2);
				assertEquals(stack.hasDataLeft(), true);

				assertEquals(stack.pop(), 2);
				assertEquals(stack.dataLeft(), 1);
				assertEquals(stack.hasDataLeft(), true);

				assertEquals(stack.pop(), 1);
				assertEquals(stack.dataLeft(), 0);
				assertEquals(stack.hasDataLeft(), false);
			}

			{
				Stack<Person> stack;
				assertEquals(stack.dataLeft(), 0);
				assertEquals(stack.hasDataLeft(), false);

				Person p("APerson", "AStr", 18);
				assertEquals(p.name, "APerson");
				assertEquals(p.address, "AStr");
				assertEquals(p.age, 18);

				stack.push(p);
				assertEquals(stack.dataLeft(), 1);
				assertEquals(stack.hasDataLeft(), true);

				Person p2 = stack.peek();
				assertEquals(p2.name, "APerson");
				assertEquals(p2.address, "AStr");
				assertEquals(p2.age, 18);

				p.name = "BPerson";
				p.address = "BStr";
				p.age = 20;

				assertEquals(p.name, "BPerson");
				assertEquals(p.address, "BStr");
				assertEquals(p.age, 20);
				assertEquals(p2.name, "APerson");
				assertEquals(p2.address, "AStr");
				assertEquals(p2.age, 18);
				assertEquals(stack.peek().name, "APerson");
				assertEquals(stack.peek().address, "AStr");
				assertEquals(stack.peek().age, 18);

				p2.name = "CPerson";
				p2.address = "CStr";
				p2.age = 21;

				assertEquals(p.name, "BPerson");
				assertEquals(p.address, "BStr");
				assertEquals(p.age, 20);
				assertEquals(p2.name, "CPerson");
				assertEquals(p2.address, "CStr");
				assertEquals(p2.age, 21);
				assertEquals(stack.peek().name, "APerson");
				assertEquals(stack.peek().address, "AStr");
				assertEquals(stack.peek().age, 18);

				stack.peek().name = "DPerson";	//This does not change anything! The change to name does not affect the next peek() call.

				assertEquals(p.name, "BPerson");
				assertEquals(p.address, "BStr");
				assertEquals(p.age, 20);
				assertEquals(p2.name, "CPerson");
				assertEquals(p2.address, "CStr");
				assertEquals(p2.age, 21);
				assertEquals(stack.peek().name, "APerson");
				assertEquals(stack.peek().address, "AStr");
				assertEquals(stack.peek().age, 18);

				stack.push(Person("EPerson", "EStr", 22));
				assertEquals(stack.dataLeft(), 2);
				assertEquals(stack.hasDataLeft(), true);
				assertEquals(stack.peek().name, "EPerson");
				assertEquals(stack.peek().address, "EStr");
				assertEquals(stack.peek().age, 22);

				stack.push(Person("FPerson", "FStr", 23));
				assertEquals(stack.dataLeft(), 3);
				assertEquals(stack.hasDataLeft(), true);
				assertEquals(stack.peek().name, "FPerson");
				assertEquals(stack.peek().address, "FStr");
				assertEquals(stack.peek().age, 23);

				stack.push(Person("GPerson", "GStr", 24));
				assertEquals(stack.dataLeft(), 4);
				assertEquals(stack.hasDataLeft(), true);
				assertEquals(stack.peek().name, "GPerson");
				assertEquals(stack.peek().address, "GStr");
				assertEquals(stack.peek().age, 24);

				stack.push(Person("HPerson", "HStr", 25));
				assertEquals(stack.dataLeft(), 5);
				assertEquals(stack.hasDataLeft(), true);
				assertEquals(stack.peek().name, "HPerson");
				assertEquals(stack.peek().address, "HStr");
				assertEquals(stack.peek().age, 25);

				p = stack.pop();
				assertEquals(stack.dataLeft(), 4);
				assertEquals(stack.hasDataLeft(), true);
				assertEquals(p.name, "HPerson");
				assertEquals(p.address, "HStr");
				assertEquals(p.age, 25);

				p = stack.pop();
				assertEquals(stack.dataLeft(), 3);
				assertEquals(stack.hasDataLeft(), true);
				assertEquals(p.name, "GPerson");
				assertEquals(p.address, "GStr");
				assertEquals(p.age, 24);

				p = stack.pop();
				assertEquals(stack.dataLeft(), 2);
				assertEquals(stack.hasDataLeft(), true);
				assertEquals(p.name, "FPerson");
				assertEquals(p.address, "FStr");
				assertEquals(p.age, 23);

				p = stack.pop();
				assertEquals(stack.dataLeft(), 1);
				assertEquals(stack.hasDataLeft(), true);
				assertEquals(p.name, "EPerson");
				assertEquals(p.address, "EStr");
				assertEquals(p.age, 22);

				p = stack.pop();
				assertEquals(stack.dataLeft(), 0);
				assertEquals(stack.hasDataLeft(), false);
				assertEquals(p.name, "APerson");
				assertEquals(p.address, "AStr");
				assertEquals(p.age, 18);
			}
		}
	}
}