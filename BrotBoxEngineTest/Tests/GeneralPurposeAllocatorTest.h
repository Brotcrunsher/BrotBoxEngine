#pragma once

#include "BBE/GeneralPurposeAllocator.h"
#include "BBE/UtilDebug.h"
#include "BBE/Random.h"

namespace bbe {
	namespace test {
		void testGeneralPurposeAllocator() {

			{
				GeneralPurposeAllocator gpa(10000);

				auto f1 = gpa.allocateObjects<float>(20);
				auto f2 = gpa.allocateObjects<float>(50);


				for (int i = 0; i < 20; i++) {
					f1[i] = (float)(i + 2);
				}
				for (int i = 0; i < 50; i++) {
					f2[i] = (float)(i + 200);
				}


				for (int i = 0; i < 20; i++) {
					assertEquals(f1[i], i + 2);
				}
				for (int i = 0; i < 50; i++) {
					assertEquals(f2[i], i + 200);
				}

				gpa.deallocate(f1);
				gpa.deallocate(f2);
			}
			

			{
				GeneralPurposeAllocator gpa(sizeof(float) * 128);
				List<GeneralPurposeAllocator::GeneralPurposeAllocatorPointer<float>> list;
				for (int i = 0; i < 64; i++)
				{
					list.add(gpa.allocateObject<float>());
				}

				for (int i = 0; i < 64; i++)
				{
					gpa.deallocate(list[i]);
				}
			}

			{
				GeneralPurposeAllocator gpa(sizeof(float) * 128);
				List<GeneralPurposeAllocator::GeneralPurposeAllocatorPointer<float>> list;
				bbe::String history("+ -0 + -0 + + -1 + -0 -0 + + -1 + -0 -0 + -0 + -0 + -0 + -0 + -0 + -0 + + -1 + + + -3 + + -3 -2 + + -3 + -0 + -0 + + -5 + + -1 -4 -0 -0 + -0 + + + -1 -1 + -0 + -0 + + -0 -2 + -3 + + + -4 -2 + + + -2 -5 -1 + + + -5 + + -4 + -8 -1 -5 -1 -2 + -2 + + -2 -2 -1 + + + + -3 + -1 + -2 -3 + + + -7 + + -8 + + + + -5 + + -5 + + -3 -7 -0 -4 + -10 -4 + -1 + + -7 -4 -2 -6 -2 + -5 + + + -7 + -7 -6 + -5 -2 -1 -2 -3 + + + -0 + + + + + + + -5 + + + -12 -7 + -4 + -12 + + -3 + -5 + -9 -5 + -3 + + -1 -12 + + + + + + + -7 + + + + -17 -1 + + + -19 -21 + -8 -5 -16 + -16 + + -15 -11 -8 -10 -7 + + + -7 + + + -9 -12 -5 + -4 + -9 -0 + -15 -8 + -8 -5 -12 -10 -1 -10 -10 + -1 -4 + + -1 -2 + -4 -4 + -0 -6 + + -2 + + + -1 + + + + -0 + -0 + -3 + + + + -16 + -8 -8 + + -2 -12 -8 -11 -11 + + -8 + -2 + + -1 -3 -12 -2 -4 -8 -5 + -2 + -0 -3 + + + -0 -3 -6 -6 + + + + + -10 + -0 + -0 + + + -0 -0 -1 + + -1 + + + + -7 + + + + + -16 -15 -6 + -13 + + + -14 -15 + -2 + -10 + -0 -13 + + -6 + + + -2 -4 + + + -14 + -2 -1 + -15 + -9 + + + -5 -10 + -6 + -6 -18 + + + -20 + + + + -3 + + + + -11 + -18 + + + -9 -22 -17 + + + + + + + -9 + + -18 -23 + + + + + -34 + -37 -2 + + + -31 + + + -37 -15 + -8 + + -18 + -13 -15 + + -31 -2 + -12 + + -2 + + + + -17 -42 -34 -26 + -29 + + -13 -39 + -10 + -2 -5 + -11 + + + -26 -7 -9 + + + -8 + -9 + -13 + + + + -29 -43 + -42 -35 + + + + + + + + + -36 + -49 + -39 + + -48 -38 + -31 -30 + -21 -13 + + + + -10 + -51 -50 -37 -13 -37 + + -8 + -10 -23 -17 + -12 -40 -25 -35 -27 -23 -1 -13 + + + + -37 + -29 + + + -44 + + -8 -12 + + -28 -44 -9 + -18 + + -6 -34 + -38 -11 -25 -10 -32 + -10 + -17 + + + -5 -11 + + + -21 -19 + + + -14 + + -34 + -13 + + -8 -19 + + + + + -21 -2 + -26 -36 + -41 + + -17 + -11 + -42 + -24 -18 -23 + + -41 -42 + -8 + -22 -39 + + -45 + -14 + + + -38 + + + + -32 -30 + -13 + + -37 + + -38 -50 + -6 -48 -19 + + -8 -17 + + -40 + -31 + -11 + + + + + + + -35 + -41 + + -0 + -3 -48 + -25 + -55 -49 -4 + + + -50 + + -10 + -19 + + -46 -36 -49 + + + + -34 + -5 -22 -53 -35 + -51 + + + + + + + + -56 -44 + + + -9 + -32 -16 + + -57 + -30 -48 -55 -6 + + + + -20 -12 + + -47 + -44 + -40 + -9 + -13 + -62 -60 + -14 + -56 + -52 + -41 -21 -29 -38 + + + -53 + + + -19 + -29 -14 -5 -52 + -11 -57 + -30 + + -53 + -14 + -51 + + -10 + -30 -38 -58 -53 -10 -38 + -25 + + + + + + -60 + + -46 + -42 + + -32 + -46 + -5 + -31 -56 -51 -38 -19 + + -60 -19 -30 + -7 -39 -36 -4 -48 -5 + + + -25 + -24 + -19 -18 + -32 -34 -16 + + + + + + + -28 -15 -46 -55 -26 -7 + + + + -39 -39 -4 + -39 + + -41 -10 -35 + + + + -30 + + -31 -16 + -27 + + + -31 -38 -52 + -14 -32 + -40 + -39 + + -38 -10 + -49 -42 + + -23 + -52 -5 + -30 + -21 -29 + + + + + -42 -30 -21 + -12 + + + + -54 + + + + + + -48 + -30 + -53 -58 + + -42 + -48 + -38 -19 + -34");
				auto splits = history.split(" ");
				for(std::size_t i = 0; i<splits.getLength(); i++)
				{
					if (splits[i][0] == L'+')
					{
						list.add(gpa.allocateObject<float>());
					}
					else
					{
						bbe::String number_s = splits[i];
						number_s.substring(1);
						int number_i =  number_s.toLong();
						gpa.deallocate(list[number_i]);
						list.removeIndex(number_i);
					}
				}

				while(list.getLength() > 0){
					gpa.deallocate(list[0]);
					list.removeIndex(0);
				}
			}


			for(int i = 0; i<1; i++)
			{
				GeneralPurposeAllocator gpa(sizeof(float) * 128);
				List<GeneralPurposeAllocator::GeneralPurposeAllocatorPointer<float>> list;
				Random rand;
				bool deallocated[1024 * 128];
				memset(deallocated, 0, sizeof(bool) * 1024 * 128);
				bbe::String history;

				for (int i = 0; i < 1024 * 128; i++)
				{
					if (rand.randomBool())
					{
						if (list.getLength() < 64)
						{
							history += "+ ";
							auto addr = gpa.allocateObject<float>();
							*addr = i;
							list.add(addr);
						}
					}
					else
					{
						if (list.getLength() > 0)
						{
							size_t index = (size_t)rand.randomInt((int)list.getLength());
							history += "-";
							history += index;
							history += " ";
							float savedValue = *list[index];
							int savedValue_i = (int)savedValue;
							assertEquals(deallocated[savedValue_i], false);
							assertGreaterThan(savedValue_i, -1);
							assertLessThan(savedValue_i, 1024 * 128);
							deallocated[savedValue_i] = true;
							gpa.deallocate(list[index]);
							list.removeIndex(index);
						}
					}
				}

				while(list.getLength() > 0){
					gpa.deallocate(list[0]);
					list.removeIndex(0);
				}
			}

			{
				GeneralPurposeAllocator gpa;

				auto f1 = gpa.allocateObjects<Person>();

				f1->print();

				gpa.deallocate(f1);
			}

		}
	}
}