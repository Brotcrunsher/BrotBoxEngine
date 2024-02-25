#include "gtest/gtest.h"
#include "BBE/BrotBoxEngine.h"
#include "TestUtils.h"

TEST(String, TotalStringTest)
{
	ASSERT_EQ((int)bbe::utf8len(""), 0);					//Simple!
	ASSERT_EQ((int)bbe::utf8len("a"), 1);					//A bit harder!
	ASSERT_EQ((int)bbe::utf8len("BrotBoxEngine!"), 14);	//Still normal...
	ASSERT_EQ((int)bbe::utf8len("αβγδ"), 4);				//Okay...
	ASSERT_EQ((int)bbe::utf8len("Großmütterchäään"), 16);	//Get ready!
	ASSERT_EQ((int)bbe::utf8len("💣🍣💃"), 3);			//God damn, I bet this line will break a few compilers... or git! 🤣

	ASSERT_EQ((int)bbe::utf8charlen(""), 1);
	ASSERT_EQ((int)bbe::utf8charlen("a"), 1);
	ASSERT_EQ((int)bbe::utf8charlen("aaaa"), 1);

	ASSERT_EQ((int)bbe::utf8charlen(""), 1);
	ASSERT_EQ((int)bbe::utf8charlen("B"), 1);
	ASSERT_EQ((int)bbe::utf8charlen("α"), 2);
	ASSERT_EQ((int)bbe::utf8charlen("β"), 2);
	ASSERT_EQ((int)bbe::utf8charlen("γ"), 2);
	ASSERT_EQ((int)bbe::utf8charlen("δ"), 2);
	ASSERT_EQ((int)bbe::utf8charlen("ß"), 2);
	ASSERT_EQ((int)bbe::utf8charlen("ä"), 2);
	ASSERT_EQ((int)bbe::utf8charlen("ö"), 2);
	ASSERT_EQ((int)bbe::utf8charlen("ü"), 2);
	ASSERT_EQ((int)bbe::utf8charlen("💣"), 4);
	ASSERT_EQ((int)bbe::utf8charlen("🍣"), 4);
	ASSERT_EQ((int)bbe::utf8charlen("💃"), 4);
	ASSERT_EQ((int)bbe::utf8charlen("\uFEFF"), 3);
	try
	{
		bbe::utf8charlen("💃" + 1); //This should create an exception.
		bbe::debugBreak();
	}
	catch (const bbe::NotStartOfUtf8Exception&)
	{
		//Do nothing, everything worked as expected.
	}

	{
		char data[] = "a";
		ASSERT_EQ(true, bbe::utf8IsSameChar("a", data));
		ASSERT_EQ(true, bbe::utf8IsSameChar(data, "a"));
		ASSERT_EQ(false, bbe::utf8IsSameChar("b", data));
		ASSERT_EQ(false, bbe::utf8IsSameChar(data, "b"));
	}
	{
		char data[] = "💣";
		ASSERT_EQ(true, bbe::utf8IsSameChar("💣", data));
		ASSERT_EQ(true, bbe::utf8IsSameChar(data, "💣"));
		ASSERT_EQ(false, bbe::utf8IsSameChar("b", data));
		ASSERT_EQ(false, bbe::utf8IsSameChar(data, "b"));
	}

	ASSERT_EQ(true, bbe::utf8IsWhitespace(" "));
	ASSERT_EQ(true, bbe::utf8IsWhitespace("\t"));
	ASSERT_EQ(true, bbe::utf8IsWhitespace("\n"));
	ASSERT_EQ(true, bbe::utf8IsWhitespace("\uFEFF"));
	ASSERT_EQ(false, bbe::utf8IsWhitespace("a"));


	//TODO add non SSO Tests
	bbe::String emptyString;
	bbe::String stringWChar("Hallo WChar");
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

	ASSERT_EQ(emptyString.getLength(), 0);
	ASSERT_EQ(stringWChar.getLength(), 11);
	ASSERT_EQ(stringClassic.getLength(), 13);
	ASSERT_EQ(stringNumber.getLength(), 11);
	ASSERT_EQ(stringCopyConstructor.getLength(), 11);
	ASSERT_EQ(stringMovedTo.getLength(), 16);
	ASSERT_EQ(stringAssignmentFrom.getLength(), 19);
	ASSERT_EQ(stringAssignmentTo.getLength(), 19);
	ASSERT_EQ(stringMoveAssignmentTo.getLength(), 24);

	{
		bbe::String stringAdd1("con");
		bbe::String stringAdd2("cattttt");
		bbe::String stringAdd3 = stringAdd1 + stringAdd2;
		ASSERT_EQ(stringAdd3, "concattttt");
	}

	{
		bbe::String stringAdd0WOSSO("Kurz ");
		bbe::String stringAdd1WOSSO("Hallo Welt! Das ist ein langer Text! ");
		bbe::String stringAddr = stringAdd0WOSSO + stringAdd1WOSSO;
		ASSERT_EQ(stringAddr, "Kurz Hallo Welt! Das ist ein langer Text! ");
		bbe::String stringAdd2WOSSO("Und hierdurch wird er sogar noch länger!");
		bbe::String stringAdd3WOSSO = stringAdd1WOSSO + stringAdd2WOSSO;
		ASSERT_EQ(stringAdd3WOSSO, "Hallo Welt! Das ist ein langer Text! Und hierdurch wird er sogar noch länger!");
	}

	{
		bbe::String stringAdd1("con");
		bbe::String stringAdd2("");
		bbe::String stringAdd3 = stringAdd1 + stringAdd2;
		ASSERT_EQ(stringAdd3, "con");
	}

	{
		bbe::String stringAdd1("");
		bbe::String stringAdd2("con");
		bbe::String stringAdd3 = stringAdd1 + stringAdd2;
		ASSERT_EQ(stringAdd3, "con");
	}

	{
		bbe::String stringAdd1("con");
		bbe::String stringAdd3 = stringAdd1 + "cattttt";
		ASSERT_EQ(stringAdd3, "concattttt");
	}

	{
		bbe::String stringAdd1("con");
		bbe::String stringAdd3 = stringAdd1 + "cattttt";
		ASSERT_EQ(stringAdd3, "concattttt");
	}

	{
		bbe::String stringAdd1("con");
		bbe::String stringAdd2("cattttt");
		bbe::String stringAdd3 = stringAdd2 + stringAdd1;
		ASSERT_EQ(stringAdd3, "catttttcon");
	}

	{
		bbe::String stringAdd1("con");
		bbe::String stringAdd3 = "cattttt" + stringAdd1;
		ASSERT_EQ(stringAdd3, "catttttcon");
	}

	{
		bbe::String stringAdd1("con");
		bbe::String stringAdd3 = "cattttt" + stringAdd1;
		ASSERT_EQ(stringAdd3, "catttttcon");
	}

	{
		bbe::String s1("A");
		s1 += "B";
		ASSERT_EQ(s1, "AB");
		s1 += "C";
		ASSERT_EQ(s1, "ABC");
		s1 += "LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL"; // <- SSO Buster
		ASSERT_EQ(s1, "ABCLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL");
	}

	{
		bbe::String stringAddTest("con");
		stringAddTest += "cattt";
		ASSERT_EQ(stringAddTest, "concattt");
	}

	{
		bbe::String stringAddTest("con");
		stringAddTest += "cattt";
		ASSERT_EQ(stringAddTest, "concattt");
	}

	{
		bbe::String stringAddTest("con");
		stringAddTest += bbe::String("cattt");
		ASSERT_EQ(stringAddTest, "concattt");
	}

	{
		bbe::String stringAddTest("con");
		stringAddTest += 17;
		ASSERT_EQ(stringAddTest, "con17");
	}

	{
		bbe::String stringTrim("   This is gonna get trimmed! :)          ");
		stringTrim.trimInPlace();
		ASSERT_EQ(stringTrim, "This is gonna get trimmed! :)");
	}

	{
		bbe::String stringTrim("   This is gonna get trimmed! :)");
		stringTrim.trimInPlace();
		ASSERT_EQ(stringTrim, "This is gonna get trimmed! :)");
	}

	{
		bbe::String stringTrim("This is gonna get trimmed! :)                      ");
		stringTrim.trimInPlace();
		ASSERT_EQ(stringTrim, "This is gonna get trimmed! :)");
	}

	{
		bbe::String stringTrim("This is gonna get trimmed! :)");
		stringTrim.trimInPlace();
		ASSERT_EQ(stringTrim, "This is gonna get trimmed! :)");
	}

	{
		bbe::String stringTrim("💣🍣💃");
		stringTrim.trimInPlace();
		ASSERT_EQ(stringTrim, "💣🍣💃");
	}

	{
		bbe::String stringTrim("          ");
		stringTrim.trimInPlace();
		ASSERT_EQ(stringTrim, "");
	}

	{
		bbe::String countTest("Some stuff in this string will get counted! this is gonna be cool!");
		ASSERT_EQ(countTest.count(bbe::String("s")), 5);
		ASSERT_EQ(countTest.count(bbe::String("")), 0);
		ASSERT_EQ(countTest.count(bbe::String(" ")), 12);
		ASSERT_EQ(countTest.count(bbe::String("this")), 2);
		ASSERT_EQ(countTest.count("s"), 5);
		ASSERT_EQ(countTest.count(""), 0);
		ASSERT_EQ(countTest.count(" "), 12);
		ASSERT_EQ(countTest.count("this"), 2);
		ASSERT_EQ(countTest.count("s"), 5);
		ASSERT_EQ(countTest.count(""), 0);
		ASSERT_EQ(countTest.count(" "), 12);
		ASSERT_EQ(countTest.count("this"), 2);
	}

	{
		bbe::String splitter("This string will be  splitted!");
		auto splitSpace = splitter.split(" ");
		ASSERT_EQ(splitSpace[0], "This");
		ASSERT_EQ(splitSpace[1], "string");
		ASSERT_EQ(splitSpace[2], "will");
		ASSERT_EQ(splitSpace[3], "be");
		ASSERT_EQ(splitSpace[4], "");
		ASSERT_EQ(splitSpace[5], "splitted!");
		ASSERT_EQ(splitSpace.getLength(), 6);



		auto splitTwoChar = splitter.split(" s");
		ASSERT_EQ(splitTwoChar[0], "This");
		ASSERT_EQ(splitTwoChar[1], "tring will be ");
		ASSERT_EQ(splitTwoChar[2], "plitted!");
		ASSERT_EQ(splitTwoChar.getLength(), 3);

		auto splitNotHappening = splitter.split("This is no part of the string");
		ASSERT_EQ(splitNotHappening[0], "This string will be  splitted!");
		ASSERT_EQ(splitNotHappening.getLength(), 1);
	}

	{
		bbe::String splitter("This string will be  splitted!");
		auto splitSpace = splitter.split(" ");
		ASSERT_EQ(splitSpace[0], "This");
		ASSERT_EQ(splitSpace[1], "string");
		ASSERT_EQ(splitSpace[2], "will");
		ASSERT_EQ(splitSpace[3], "be");
		ASSERT_EQ(splitSpace[4], "");
		ASSERT_EQ(splitSpace[5], "splitted!");
		ASSERT_EQ(splitSpace.getLength(), 6);

		auto splitTwoChar = splitter.split(" s");
		ASSERT_EQ(splitTwoChar[0], "This");
		ASSERT_EQ(splitTwoChar[1], "tring will be ");
		ASSERT_EQ(splitTwoChar[2], "plitted!");
		ASSERT_EQ(splitTwoChar.getLength(), 3);

		auto splitNotHappening = splitter.split("This is no part of the string");
		ASSERT_EQ(splitNotHappening[0], "This string will be  splitted!");
		ASSERT_EQ(splitNotHappening.getLength(), 1);
	}

	{
		bbe::String containString("This string will be analyzed if it contains various stuff");
		ASSERT_EQ(containString.contains(" "), true);
		ASSERT_EQ(containString.contains("will"), true);
		ASSERT_EQ(containString.contains("t c"), true);
		ASSERT_EQ(containString.contains("apple"), false);
		ASSERT_EQ(containString.contains("not contained"), false);
	}

	{
		bbe::String containString("This string will be analyzed if it contains various stuff");
		ASSERT_EQ(containString.contains(" "), true);
		ASSERT_EQ(containString.contains("will"), true);
		ASSERT_EQ(containString.contains("t c"), true);
		ASSERT_EQ(containString.contains("apple"), false);
		ASSERT_EQ(containString.contains("not contained"), false);
	}

	{
		bbe::String searchString("This string will be for searched through for various words!");
		ASSERT_EQ(searchString.search("This"), 0);
		ASSERT_EQ(searchString.search(""), 0);
		ASSERT_EQ(searchString.search("s"), 3);
		ASSERT_EQ(searchString.search("for"), 20);
		ASSERT_EQ(searchString.search("not contained"), -1);
	}

	{
		bbe::String searchString("This string will be for searched through for various words!");
		ASSERT_EQ(searchString.search("This"), 0);
		ASSERT_EQ(searchString.search(""), 0);
		ASSERT_EQ(searchString.search("s"), 3);
		ASSERT_EQ(searchString.search("for"), 20);
		ASSERT_EQ(searchString.search("not contained"), -1);
	}

	{
		bbe::String searchString("This string will be for searched through for various words!");
		ASSERT_EQ(searchString.search(bbe::String("This")), 0);
		ASSERT_EQ(searchString.search(bbe::String("")), 0);
		ASSERT_EQ(searchString.search(bbe::String("s")), 3);
		ASSERT_EQ(searchString.search(bbe::String("for")), 20);
		ASSERT_EQ(searchString.search(bbe::String("not contained")), -1);
	}

	{
		bbe::String numberString("1928");
		ASSERT_EQ(numberString.toLong(), 1928);
		ASSERT_EQ(numberString.toDouble(), 1928);
		ASSERT_EQ(numberString.toFloat(), 1928);
	}

	{
		bbe::String numberString("1928.5");
		ASSERT_EQ(numberString.toLong(), 1928);
		ASSERT_EQ(numberString.toDouble(), 1928.5);
		ASSERT_EQ(numberString.toFloat(), 1928.5);
	}

	{
		bbe::String numberString("not a number");
		ASSERT_EQ(numberString.toLong(), 0);
		ASSERT_EQ(numberString.toDouble(), 0);
		ASSERT_EQ(numberString.toFloat(), 0);
	}

	{
		bbe::String numberString("number with offset 1282.5");
		ASSERT_EQ(numberString.toLong(), 0);
		ASSERT_EQ(numberString.toDouble(), 0);
		ASSERT_EQ(numberString.toFloat(), 0);
	}

	{
		bbe::String numberString("1282.5 number was at the beginning");
		ASSERT_EQ(numberString.toLong(), 1282);
		ASSERT_EQ(numberString.toDouble(), 1282.5);
		ASSERT_EQ(numberString.toFloat(), 1282.5);
	}

	{
		bbe::String numberString("     1282.5 number was at the beginning even space is okay");
		ASSERT_EQ(numberString.toLong(), 1282);
		ASSERT_EQ(numberString.toDouble(), 1282.5);
		ASSERT_EQ(numberString.toFloat(), 1282.5);
	}

	{
		bbe::String operatorString("working with operator[] :)");
		ASSERT_EQ(operatorString[1], L'o');
		ASSERT_EQ(operatorString[5], L'n');
		ASSERT_EQ(operatorString[7], L' ');
		ASSERT_EQ(operatorString[10], L't');
		ASSERT_EQ(operatorString[13], L'o');
	}

	{
		bbe::String lowerUpperShifter("ThIs stRing will swiTCH BeTWeen lowER and UPPer cAse!");
		lowerUpperShifter.toLowerCaseInPlace();
		ASSERT_EQ(lowerUpperShifter, "this string will switch between lower and upper case!");
		lowerUpperShifter.toLowerCaseInPlace();
		ASSERT_EQ(lowerUpperShifter, "this string will switch between lower and upper case!");
		lowerUpperShifter.toUpperCaseInPlace();
		ASSERT_EQ(lowerUpperShifter, "THIS STRING WILL SWITCH BETWEEN LOWER AND UPPER CASE!");
		lowerUpperShifter.toUpperCaseInPlace();
		ASSERT_EQ(lowerUpperShifter, "THIS STRING WILL SWITCH BETWEEN LOWER AND UPPER CASE!");
		lowerUpperShifter.toLowerCaseInPlace();
		ASSERT_EQ(lowerUpperShifter, "this string will switch between lower and upper case!");
		lowerUpperShifter.toUpperCaseInPlace();
		ASSERT_EQ(lowerUpperShifter, "THIS STRING WILL SWITCH BETWEEN LOWER AND UPPER CASE!");
	}

	ASSERT_EQ(stringWChar, bbe::String("Hallo WChar"));
	ASSERT_EQ(bbe::String("Hallo WChar"), stringWChar);
	ASSERT_NE(stringWChar, bbe::String("Hallo WChaa"));
	ASSERT_NE(bbe::String("Haloo WChar"), stringWChar);

	ASSERT_EQ(emptyString, "");
	ASSERT_EQ("", emptyString);
	ASSERT_NE(emptyString, "a");
	ASSERT_NE("a", emptyString);
	ASSERT_EQ(emptyString, "");
	ASSERT_EQ("", emptyString);
	ASSERT_NE(emptyString, "a");
	ASSERT_NE("a", emptyString);

	ASSERT_EQ(stringWChar, "Hallo WChar");
	ASSERT_EQ("Hallo WChar", stringWChar);
	ASSERT_NE(stringWChar, "HalloWChar");
	ASSERT_NE("HalloWChar", stringWChar);
	ASSERT_EQ(stringWChar, "Hallo WChar");
	ASSERT_EQ("Hallo WChar", stringWChar);
	ASSERT_NE(stringWChar, "HalloWChar");
	ASSERT_NE("HalloWChar", stringWChar);

	ASSERT_EQ(stringClassic, "Hallo Classic");
	ASSERT_EQ("Hallo Classic", stringClassic);
	ASSERT_NE(stringClassic, "HalloClassic");
	ASSERT_NE("HalloClassic", stringClassic);
	ASSERT_EQ(stringClassic, "Hallo Classic");
	ASSERT_EQ("Hallo Classic", stringClassic);
	ASSERT_NE(stringClassic, "HalloClassic");
	ASSERT_NE("HalloClassic", stringClassic);

	ASSERT_EQ(stringNumber, "2839.192000");
	ASSERT_EQ("2839.192000", stringNumber);
	ASSERT_NE(stringNumber, "2839.392000");
	ASSERT_NE("2839.392000", stringNumber);
	ASSERT_EQ(stringNumber, "2839.192000");
	ASSERT_EQ("2839.192000", stringNumber);
	ASSERT_NE(stringNumber, "2839.392000");
	ASSERT_NE("2839.392000", stringNumber);

	ASSERT_EQ(stringCopyConstructor, "Hallo WChar");
	ASSERT_EQ("Hallo WChar", stringCopyConstructor);
	ASSERT_NE(stringCopyConstructor, "HalloWChar");
	ASSERT_NE("HalloWChar", stringCopyConstructor);
	ASSERT_EQ(stringCopyConstructor, "Hallo WChar");
	ASSERT_EQ("Hallo WChar", stringCopyConstructor);
	ASSERT_NE(stringCopyConstructor, "HalloWChar");
	ASSERT_NE("HalloWChar", stringCopyConstructor);

	ASSERT_EQ(stringMovedTo, "I will be moved!");
	ASSERT_EQ("I will be moved!", stringMovedTo);
	ASSERT_NE(stringMovedTo, "I willbe moved!");
	ASSERT_NE("I will b moved!", stringMovedTo);
	ASSERT_EQ(stringMovedTo, "I will be moved!");
	ASSERT_EQ("I will be moved!", stringMovedTo);
	ASSERT_NE(stringMovedTo, "I will be mved!");
	ASSERT_NE("I willbe moved!", stringMovedTo);

	ASSERT_EQ(stringAssignmentTo, "I will be assigned!");
	ASSERT_EQ("I will be assigned!", stringAssignmentTo);
	ASSERT_NE(stringAssignmentTo, "I will be bssigned!");
	ASSERT_NE("I will be assjgned!", stringAssignmentTo);
	ASSERT_EQ(stringAssignmentTo, "I will be assigned!");
	ASSERT_EQ("I will be assigned!", stringAssignmentTo);
	ASSERT_NE(stringAssignmentTo, "I will be ssigned!");
	ASSERT_NE("I will be assigned", stringAssignmentTo);

	ASSERT_EQ(stringAssignmentFrom, "I will be assigned!");
	ASSERT_EQ("I will be assigned!", stringAssignmentFrom);
	ASSERT_NE(stringAssignmentFrom, "I will be bssigned!");
	ASSERT_NE("I will be assjgned!", stringAssignmentFrom);
	ASSERT_EQ(stringAssignmentFrom, "I will be assigned!");
	ASSERT_EQ("I will be assigned!", stringAssignmentFrom);
	ASSERT_NE(stringAssignmentFrom, "I will be ssigned!");
	ASSERT_NE("I will be assigned", stringAssignmentFrom);

	ASSERT_EQ(stringMoveAssignmentTo, "I will be move-assigned!");
	ASSERT_EQ("I will be move-assigned!", stringMoveAssignmentTo);
	ASSERT_NE(stringMoveAssignmentTo, "I will be move-assgned!");
	ASSERT_NE("I will be move-assignd!", stringMoveAssignmentTo);
	ASSERT_EQ(stringMoveAssignmentTo, "I will be move-assigned!");
	ASSERT_EQ("I will be move-assigned!", stringMoveAssignmentTo);
	ASSERT_NE(stringMoveAssignmentTo, "I will be moveassigned!");
	ASSERT_NE("I will be move-asigned!", stringMoveAssignmentTo);

	{
		bbe::String s1 = "aaba";
		ASSERT_EQ(s1.replace("b", ""), "aaa");
		ASSERT_EQ(s1.replace("b", "b"), "aaba");
		ASSERT_EQ(s1.replace("b", "bb"), "aabba");
		ASSERT_EQ(s1.replace("b", " "), "aa a");
		ASSERT_EQ(s1.replace("b", "  "), "aa  a");
		ASSERT_EQ(s1.replace("b", "\t"), "aa\ta");
		ASSERT_EQ(s1.replace("b", "\t\t"), "aa\t\ta");
		ASSERT_EQ(s1.replace("b", "ö"), "aaöa");
		ASSERT_EQ(s1.replace("b", "öö"), "aaööa");
	}
	{
		bbe::String s1 = "aaöa";
		ASSERT_EQ(s1.replace("ö", ""), "aaa");
		ASSERT_EQ(s1.replace("ö", "b"), "aaba");
		ASSERT_EQ(s1.replace("ö", "bb"), "aabba");
		ASSERT_EQ(s1.replace("ö", " "), "aa a");
		ASSERT_EQ(s1.replace("ö", "  "), "aa  a");
		ASSERT_EQ(s1.replace("ö", "\t"), "aa\ta");
		ASSERT_EQ(s1.replace("ö", "\t\t"), "aa\t\ta");
		ASSERT_EQ(s1.replace("ö", "ö"), "aaöa");
		ASSERT_EQ(s1.replace("ö", "öö"), "aaööa");
	}
	{
		bbe::String s1 = "aöba";
		ASSERT_EQ(s1.replace("b", ""), "aöa");
		ASSERT_EQ(s1.replace("b", "b"), "aöba");
		ASSERT_EQ(s1.replace("b", "bb"), "aöbba");
		ASSERT_EQ(s1.replace("b", " "), "aö a");
		ASSERT_EQ(s1.replace("b", "  "), "aö  a");
		ASSERT_EQ(s1.replace("b", "\t"), "aö\ta");
		ASSERT_EQ(s1.replace("b", "\t\t"), "aö\t\ta");
		ASSERT_EQ(s1.replace("b", "ö"), "aööa");
		ASSERT_EQ(s1.replace("b", "öö"), "aöööa");
	}
	{
		bbe::String s1 = "aabö";
		ASSERT_EQ(s1.replace("b", ""), "aaö");
		ASSERT_EQ(s1.replace("b", "b"), "aabö");
		ASSERT_EQ(s1.replace("b", "bb"), "aabbö");
		ASSERT_EQ(s1.replace("b", " "), "aa ö");
		ASSERT_EQ(s1.replace("b", "  "), "aa  ö");
		ASSERT_EQ(s1.replace("b", "\t"), "aa\tö");
		ASSERT_EQ(s1.replace("b", "\t\t"), "aa\t\tö");
		ASSERT_EQ(s1.replace("b", "ö"), "aaöö");
		ASSERT_EQ(s1.replace("b", "öö"), "aaööö");
	}
	{
		bbe::String s1 = "aabaö";
		ASSERT_EQ(s1.replace("b", ""), "aaaö");
		ASSERT_EQ(s1.replace("b", "b"), "aabaö");
		ASSERT_EQ(s1.replace("b", "bb"), "aabbaö");
		ASSERT_EQ(s1.replace("b", " "), "aa aö");
		ASSERT_EQ(s1.replace("b", "  "), "aa  aö");
		ASSERT_EQ(s1.replace("b", "\t"), "aa\taö");
		ASSERT_EQ(s1.replace("b", "\t\t"), "aa\t\taö");
		ASSERT_EQ(s1.replace("b", "ö"), "aaöaö");
		ASSERT_EQ(s1.replace("b", "öö"), "aaööaö");
	}
	{
		bbe::String s1 = "äöü";
		ASSERT_EQ(s1.replace("b", ""), "äöü");
		ASSERT_EQ(s1.replace("ä", ""), "öü");
		ASSERT_EQ(s1.replace("ö", ""), "äü");
		ASSERT_EQ(s1.replace("ü", ""), "äö");
		ASSERT_EQ(s1.replace("ä", "b"), "böü");
		ASSERT_EQ(s1.replace("ö", "b"), "äbü");
		ASSERT_EQ(s1.replace("ü", "b"), "äöb");
		ASSERT_EQ(s1.replace("ä", "ä"), "äöü");
		ASSERT_EQ(s1.replace("ö", "ä"), "ääü");
		ASSERT_EQ(s1.replace("ü", "ä"), "äöä");
	}

	{
		bbe::String s1 = "aaa";
		bbe::String s2 = "aaa";
		bbe::String s3 = "aab";
		bbe::String s4 = "bbb";

		ASSERT_FALSE(s1 < s2);
		ASSERT_TRUE (s1 < s3);
		ASSERT_TRUE (s1 < s4);
		ASSERT_FALSE(s2 < s1);
		ASSERT_TRUE (s2 < s3);
		ASSERT_TRUE (s2 < s4);
		ASSERT_FALSE(s3 < s1);
		ASSERT_FALSE(s3 < s2);
		ASSERT_TRUE (s3 < s4);
		ASSERT_FALSE(s4 < s1);
		ASSERT_FALSE(s4 < s2);
		ASSERT_FALSE(s4 < s3);
	}

	{
		bbe::String s1 = "aaa";
		bbe::String s2 = "aa";

		ASSERT_FALSE(s1 < s2);
		ASSERT_TRUE (s2 < s1);
	}
}
