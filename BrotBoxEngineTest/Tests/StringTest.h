#pragma once

#include "BBE/String.h"
#include <iostream>
#include <string>
#include "BBE/UtilTest.h"

namespace bbe {
	namespace test {
		void testString() {
			assertEquals((int)bbe::utf8len(u8"") , 0);					//Simple!
			assertEquals((int)bbe::utf8len(u8"a"), 1);					//A bit harder!
			assertEquals((int)bbe::utf8len(u8"BrotBoxEngine!"), 14);		//Still normal...
			assertEquals((int)bbe::utf8len(u8"αβγδ"), 4);				//Okay...
			assertEquals((int)bbe::utf8len(u8"Großmütterchäään"), 16);	//Get ready!
			assertEquals((int)bbe::utf8len(u8"💣🍣💃"), 3);				//God damn, I bet this line will break a few compilers... or git! 🤣

			assertEquals((int)bbe::utf8charlen(u8""), 1);
			assertEquals((int)bbe::utf8charlen(u8"a"), 1);
			assertEquals((int)bbe::utf8charlen(u8"aaaa"), 1);

			assertEquals((int)bbe::utf8charlen(u8""),       1);
			assertEquals((int)bbe::utf8charlen(u8"B"),      1);
			assertEquals((int)bbe::utf8charlen(u8"α"),      2);
			assertEquals((int)bbe::utf8charlen(u8"β"),      2);
			assertEquals((int)bbe::utf8charlen(u8"γ"),      2);
			assertEquals((int)bbe::utf8charlen(u8"δ"),      2);
			assertEquals((int)bbe::utf8charlen(u8"ß"),      2);
			assertEquals((int)bbe::utf8charlen(u8"ä"),      2);
			assertEquals((int)bbe::utf8charlen(u8"ö"),      2);
			assertEquals((int)bbe::utf8charlen(u8"ü"),      2);
			assertEquals((int)bbe::utf8charlen(u8"💣"),     4);
			assertEquals((int)bbe::utf8charlen(u8"🍣"),     4);
			assertEquals((int)bbe::utf8charlen(u8"💃"),     4);
			assertEquals((int)bbe::utf8charlen(u8"\uFEFF"), 3);
			try
			{
				bbe::utf8charlen(u8"💃" + 1); //This should create an exception.
				debugBreak();
			}
			catch (bbe::NotStartOfUtf8Exception e)
			{
				//Do nothing, everything worked as expected.
			}

			{
				char data[] = u8"a";
				assertEquals(true, bbe::utf8IsSameChar(u8"a", data));
				assertEquals(true, bbe::utf8IsSameChar(data, u8"a"));
				assertEquals(false, bbe::utf8IsSameChar(u8"b", data));
				assertEquals(false, bbe::utf8IsSameChar(data, u8"b"));
			}
			{
				char data[] = u8"💣";
				assertEquals(true, bbe::utf8IsSameChar(u8"💣", data));
				assertEquals(true, bbe::utf8IsSameChar(data, u8"💣"));
				assertEquals(false, bbe::utf8IsSameChar(u8"b", data));
				assertEquals(false, bbe::utf8IsSameChar(data, u8"b"));
			}

			assertEquals(true, bbe::utf8IsWhitespace(u8" "));
			assertEquals(true, bbe::utf8IsWhitespace(u8"\t"));
			assertEquals(true, bbe::utf8IsWhitespace(u8"\n"));
			assertEquals(true, bbe::utf8IsWhitespace(u8"\uFEFF"));
			assertEquals(false, bbe::utf8IsWhitespace(u8"a"));


			//TODO add non SSO Tests
			bbe::String emptyString;
			bbe::String stringWChar  ("Hallo WChar");
			bbe::String stringClassic("Hallo Classic");
			bbe::String stringNumber(2839.192);

			bbe::String stringCopyConstructor(stringWChar);
			bbe::String stringMovedFrom("I will be moved!");
			bbe::String stringMovedTo(std::move(stringMovedFrom));

			bbe::String stringAssignmentFrom("I will be assigned!");
			bbe::String stringAssignmentTo("");
			stringAssignmentTo = stringAssignmentFrom;

			bbe::String stringMoveAssignmentFrom("I will be move-assigned!");
			bbe::String stringMoveAssignmentTo("");
			stringMoveAssignmentTo = std::move(stringMoveAssignmentFrom);

			assertEquals(emptyString.getLength()           , 0);
			assertEquals(stringWChar.getLength()           , 11);
			assertEquals(stringClassic.getLength()         , 13);
			assertEquals(stringNumber.getLength()          , 11);
			assertEquals(stringCopyConstructor.getLength() , 11);
			assertEquals(stringMovedTo.getLength()         , 16);
			assertEquals(stringAssignmentFrom.getLength()  , 19);
			assertEquals(stringAssignmentTo.getLength()    , 19);
			assertEquals(stringMoveAssignmentTo.getLength(), 24);

			{
				bbe::String stringAdd1("con");
				bbe::String stringAdd2("cattttt");
				bbe::String stringAdd3 = stringAdd1 + stringAdd2;
				assertEquals(stringAdd3, "concattttt");
			}

			{
				bbe::String stringAdd0WOSSO("Kurz ");
				bbe::String stringAdd1WOSSO("Hallo Welt! Das ist ein langer Text! ");
				bbe::String stringAddr = stringAdd0WOSSO + stringAdd1WOSSO;
				assertEquals(stringAddr, "Kurz Hallo Welt! Das ist ein langer Text! ");
				bbe::String stringAdd2WOSSO("Und hierdurch wird er sogar noch länger!");
				bbe::String stringAdd3WOSSO = stringAdd1WOSSO + stringAdd2WOSSO;
				assertEquals(stringAdd3WOSSO, "Hallo Welt! Das ist ein langer Text! Und hierdurch wird er sogar noch länger!");
			}

			{
				bbe::String stringAdd1("con");
				bbe::String stringAdd2("");
				bbe::String stringAdd3 = stringAdd1 + stringAdd2;
				assertEquals(stringAdd3, "con");
			}

			{
				bbe::String stringAdd1("");
				bbe::String stringAdd2("con");
				bbe::String stringAdd3 = stringAdd1 + stringAdd2;
				assertEquals(stringAdd3, "con");
			}

			{
				bbe::String stringAdd1("con");
				bbe::String stringAdd3 = stringAdd1 + "cattttt";
				assertEquals(stringAdd3, "concattttt");
			}

			{
				bbe::String stringAdd1("con");
				bbe::String stringAdd3 = stringAdd1 + "cattttt";
				assertEquals(stringAdd3, "concattttt");
			}

			{
				bbe::String stringAdd1("con");
				bbe::String stringAdd2("cattttt");
				bbe::String stringAdd3 = stringAdd2 + stringAdd1;
				assertEquals(stringAdd3, "catttttcon");
			}

			{
				bbe::String stringAdd1("con");
				bbe::String stringAdd3 = "cattttt" + stringAdd1;
				assertEquals(stringAdd3, "catttttcon");
			}

			{
				bbe::String stringAdd1("con");
				bbe::String stringAdd3 = "cattttt" + stringAdd1;
				assertEquals(stringAdd3, "catttttcon");
			}

			{
				bbe::String s1("A");
				s1 += "B";
				assertEquals(s1, "AB");
				s1 += "C";
				assertEquals(s1, "ABC");
				s1 += "LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL"; // <- SSO Buster
				assertEquals(s1, "ABCLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL");
			}

			{
				bbe::String stringAddTest("con");
				stringAddTest += "cattt";
				assertEquals(stringAddTest, "concattt");
			}

			{
				bbe::String stringAddTest("con");
				stringAddTest += "cattt";
				assertEquals(stringAddTest, "concattt");
			}

			{
				bbe::String stringAddTest("con");
				stringAddTest += String("cattt");
				assertEquals(stringAddTest, "concattt");
			}

			{
				bbe::String stringAddTest("con");
				stringAddTest += 17;
				assertEquals(stringAddTest, "con17");
			}
			
			{
				bbe::String stringTrim("   This is gonna get trimmed! :)          ");
				stringTrim.trim();
				assertEquals(stringTrim, "This is gonna get trimmed! :)");
			}

			{
				bbe::String stringTrim("   This is gonna get trimmed! :)");
				stringTrim.trim();
				assertEquals(stringTrim, "This is gonna get trimmed! :)");
			}

			{
				bbe::String stringTrim("This is gonna get trimmed! :)                      ");
				stringTrim.trim();
				assertEquals(stringTrim, "This is gonna get trimmed! :)");
			}

			{
				bbe::String stringTrim("This is gonna get trimmed! :)");
				stringTrim.trim();
				assertEquals(stringTrim, "This is gonna get trimmed! :)");
			}

			{
				bbe::String stringTrim("          ");
				stringTrim.trim();
				assertEquals(stringTrim, "");
			}

			{
				bbe::String countTest("Some stuff in this string will get counted! this is gonna be cool!");
				assertEquals(countTest.count(bbe::String("s")), 5);
				assertEquals(countTest.count(bbe::String("")), 0);
				assertEquals(countTest.count(bbe::String(" ")), 12);
				assertEquals(countTest.count(bbe::String("this")), 2);
				assertEquals(countTest.count("s"), 5);
				assertEquals(countTest.count(""), 0);
				assertEquals(countTest.count(" "), 12);
				assertEquals(countTest.count("this"), 2);
				assertEquals(countTest.count("s"), 5);
				assertEquals(countTest.count(""), 0);
				assertEquals(countTest.count(" "), 12);
				assertEquals(countTest.count("this"), 2);
			}

			{
				bbe::String splitter("This string will be  splitted!");
				auto splitSpace = splitter.split(" ");
				assertEquals(splitSpace[0], "This");
				assertEquals(splitSpace[1], "string");
				assertEquals(splitSpace[2], "will");
				assertEquals(splitSpace[3], "be");
				assertEquals(splitSpace[4], "");
				assertEquals(splitSpace[5], "splitted!");
				assertEquals(splitSpace.getLength(), 6);



				auto splitTwoChar = splitter.split(" s");
				assertEquals(splitTwoChar[0], "This");
				assertEquals(splitTwoChar[1], "tring will be ");
				assertEquals(splitTwoChar[2], "plitted!");
				assertEquals(splitTwoChar.getLength(), 3);

				auto splitNotHappening = splitter.split("This is no part of the string");
				assertEquals(splitNotHappening[0], "This string will be  splitted!");
				assertEquals(splitNotHappening.getLength(), 1);
			}

			{
				bbe::String splitter("This string will be  splitted!");
				auto splitSpace = splitter.split(" ");
				assertEquals(splitSpace[0], "This");
				assertEquals(splitSpace[1], "string");
				assertEquals(splitSpace[2], "will");
				assertEquals(splitSpace[3], "be");
				assertEquals(splitSpace[4], "");
				assertEquals(splitSpace[5], "splitted!");
				assertEquals(splitSpace.getLength(), 6);

				auto splitTwoChar = splitter.split(" s");
				assertEquals(splitTwoChar[0], "This");
				assertEquals(splitTwoChar[1], "tring will be ");
				assertEquals(splitTwoChar[2], "plitted!");
				assertEquals(splitTwoChar.getLength(), 3);

				auto splitNotHappening = splitter.split("This is no part of the string");
				assertEquals(splitNotHappening[0], "This string will be  splitted!");
				assertEquals(splitNotHappening.getLength(), 1);
			}

			{
				bbe::String containString("This string will be analyzed if it contains various stuff");
				assertEquals(containString.contains(" "), true);
				assertEquals(containString.contains("will"), true);
				assertEquals(containString.contains("t c"), true);
				assertEquals(containString.contains("apple"), false);
				assertEquals(containString.contains("not contained"), false);
			}

			{
				bbe::String containString("This string will be analyzed if it contains various stuff");
				assertEquals(containString.contains(" "), true);
				assertEquals(containString.contains("will"), true);
				assertEquals(containString.contains("t c"), true);
				assertEquals(containString.contains("apple"), false);
				assertEquals(containString.contains("not contained"), false);
			}

			{
				bbe::String searchString("This string will be for searched through for various words!");
				assertEquals(searchString.search("This"), 0);
				assertEquals(searchString.search(""), 0);
				assertEquals(searchString.search("s"), 3);
				assertEquals(searchString.search("for"), 20);
				assertEquals(searchString.search("not contained"), -1);
			}

			{
				bbe::String searchString("This string will be for searched through for various words!");
				assertEquals(searchString.search("This"), 0);
				assertEquals(searchString.search(""), 0);
				assertEquals(searchString.search("s"), 3);
				assertEquals(searchString.search("for"), 20);
				assertEquals(searchString.search("not contained"), -1);
			}

			{
				bbe::String searchString("This string will be for searched through for various words!");
				assertEquals(searchString.search(bbe::String("This")), 0);
				assertEquals(searchString.search(bbe::String("")), 0);
				assertEquals(searchString.search(bbe::String("s")), 3);
				assertEquals(searchString.search(bbe::String("for")), 20);
				assertEquals(searchString.search(bbe::String("not contained")), -1);
			}

			{
				bbe::String numberString("1928");
				assertEquals(numberString.toLong(), 1928);
				assertEquals(numberString.toDouble(), 1928);
				assertEquals(numberString.toFloat(), 1928);
			}

			{
				bbe::String numberString("1928.5");
				assertEquals(numberString.toLong(), 1928);
				assertEquals(numberString.toDouble(), 1928.5);
				assertEquals(numberString.toFloat(), 1928.5);
			}

			{
				bbe::String numberString("not a number");
				assertEquals(numberString.toLong(), 0);
				assertEquals(numberString.toDouble(), 0);
				assertEquals(numberString.toFloat(), 0);
			}

			{
				bbe::String numberString("number with offset 1282.5");
				assertEquals(numberString.toLong(), 0);
				assertEquals(numberString.toDouble(), 0);
				assertEquals(numberString.toFloat(), 0);
			}

			{
				bbe::String numberString("1282.5 number was at the beginning");
				assertEquals(numberString.toLong(), 1282);
				assertEquals(numberString.toDouble(), 1282.5);
				assertEquals(numberString.toFloat(), 1282.5);
			}

			{
				bbe::String numberString("     1282.5 number was at the beginning even space is okay");
				assertEquals(numberString.toLong(), 1282);
				assertEquals(numberString.toDouble(), 1282.5);
				assertEquals(numberString.toFloat(), 1282.5);
			}

			{
				bbe::String operatorString("working with operator[] :)");
				assertEquals(operatorString[1], L'o');
				assertEquals(operatorString[5], L'n');
				assertEquals(operatorString[7], L' ');
				assertEquals(operatorString[10], L't');
				assertEquals(operatorString[13], L'o');
			}

			{
				bbe::String lowerUpperShifter("ThIs stRing will swiTCH BeTWeen lowER and UPPer cAse!");
				lowerUpperShifter.toLowerCase();
				assertEquals(lowerUpperShifter, "this string will switch between lower and upper case!");
				lowerUpperShifter.toLowerCase();
				assertEquals(lowerUpperShifter, "this string will switch between lower and upper case!");
				lowerUpperShifter.toUpperCase();
				assertEquals(lowerUpperShifter, "THIS STRING WILL SWITCH BETWEEN LOWER AND UPPER CASE!");
				lowerUpperShifter.toUpperCase();
				assertEquals(lowerUpperShifter, "THIS STRING WILL SWITCH BETWEEN LOWER AND UPPER CASE!");
				lowerUpperShifter.toLowerCase();
				assertEquals(lowerUpperShifter, "this string will switch between lower and upper case!");
				lowerUpperShifter.toUpperCase();
				assertEquals(lowerUpperShifter, "THIS STRING WILL SWITCH BETWEEN LOWER AND UPPER CASE!");
			}

			assertEquals(stringWChar, String("Hallo WChar"));
			assertEquals(String("Hallo WChar"), stringWChar);
			assertUnequals(stringWChar, String("Hallo WChaa"));
			assertUnequals(String("Haloo WChar"), stringWChar);

			assertEquals  (emptyString, "");
			assertEquals  ("", emptyString);
			assertUnequals(emptyString, "a");
			assertUnequals("a", emptyString);
			assertEquals  (emptyString, "");
			assertEquals  ("", emptyString);
			assertUnequals(emptyString, "a");
			assertUnequals("a", emptyString);

			assertEquals  (stringWChar, "Hallo WChar");
			assertEquals  ("Hallo WChar", stringWChar);
			assertUnequals(stringWChar, "HalloWChar");
			assertUnequals("HalloWChar", stringWChar);
			assertEquals  (stringWChar, "Hallo WChar");
			assertEquals  ("Hallo WChar", stringWChar);
			assertUnequals(stringWChar, "HalloWChar");
			assertUnequals("HalloWChar", stringWChar);

			assertEquals  (stringClassic, "Hallo Classic");
			assertEquals  ("Hallo Classic", stringClassic);
			assertUnequals(stringClassic, "HalloClassic");
			assertUnequals("HalloClassic", stringClassic);
			assertEquals  (stringClassic, "Hallo Classic");
			assertEquals  ("Hallo Classic", stringClassic);
			assertUnequals(stringClassic, "HalloClassic");
			assertUnequals("HalloClassic", stringClassic);
			
			assertEquals  (stringNumber, "2839.192000");
			assertEquals  ("2839.192000", stringNumber);
			assertUnequals(stringNumber, "2839.392000");
			assertUnequals("2839.392000", stringNumber);
			assertEquals  (stringNumber, "2839.192000");
			assertEquals  ("2839.192000", stringNumber);
			assertUnequals(stringNumber, "2839.392000");
			assertUnequals("2839.392000", stringNumber);

			assertEquals  (stringCopyConstructor, "Hallo WChar");
			assertEquals  ("Hallo WChar", stringCopyConstructor);
			assertUnequals(stringCopyConstructor, "HalloWChar");
			assertUnequals("HalloWChar", stringCopyConstructor);
			assertEquals  (stringCopyConstructor, "Hallo WChar");
			assertEquals  ("Hallo WChar", stringCopyConstructor);
			assertUnequals(stringCopyConstructor, "HalloWChar");
			assertUnequals("HalloWChar", stringCopyConstructor);

			assertEquals  (stringMovedTo, "I will be moved!");
			assertEquals  ("I will be moved!", stringMovedTo);
			assertUnequals(stringMovedTo, "I willbe moved!");
			assertUnequals("I will b moved!", stringMovedTo);
			assertEquals  (stringMovedTo, "I will be moved!");
			assertEquals  ("I will be moved!", stringMovedTo);
			assertUnequals(stringMovedTo, "I will be mved!");
			assertUnequals("I willbe moved!", stringMovedTo);

			assertEquals  (stringAssignmentTo, "I will be assigned!");
			assertEquals  ("I will be assigned!", stringAssignmentTo);
			assertUnequals(stringAssignmentTo, "I will be bssigned!");
			assertUnequals("I will be assjgned!", stringAssignmentTo);
			assertEquals  (stringAssignmentTo, "I will be assigned!");
			assertEquals  ("I will be assigned!", stringAssignmentTo);
			assertUnequals(stringAssignmentTo, "I will be ssigned!");
			assertUnequals("I will be assigned", stringAssignmentTo);

			assertEquals  (stringAssignmentFrom, "I will be assigned!");
			assertEquals  ("I will be assigned!", stringAssignmentFrom);
			assertUnequals(stringAssignmentFrom, "I will be bssigned!");
			assertUnequals("I will be assjgned!", stringAssignmentFrom);
			assertEquals  (stringAssignmentFrom, "I will be assigned!");
			assertEquals  ("I will be assigned!", stringAssignmentFrom);
			assertUnequals(stringAssignmentFrom, "I will be ssigned!");
			assertUnequals("I will be assigned", stringAssignmentFrom);

			assertEquals  (stringMoveAssignmentTo, "I will be move-assigned!");
			assertEquals  ("I will be move-assigned!", stringMoveAssignmentTo);
			assertUnequals(stringMoveAssignmentTo, "I will be move-assgned!");
			assertUnequals("I will be move-assignd!", stringMoveAssignmentTo);
			assertEquals  (stringMoveAssignmentTo, "I will be move-assigned!");
			assertEquals  ("I will be move-assigned!", stringMoveAssignmentTo);
			assertUnequals(stringMoveAssignmentTo, "I will be moveassigned!");
			assertUnequals("I will be move-asigned!", stringMoveAssignmentTo);
		}
	}
}