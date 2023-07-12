#ifndef EKONYV_DATABASE_TEST_H
#define EKONYV_DATABASE_TEST_H

#include "../src/storage/database.h"
#include "../src/storage/storage.h"
#include "../src/string/to_string.h"

#define REQUIRE(cond)                                               \
	if (!(cond)) {                                                  \
		Serial.println("Test error: " #cond " evaluated to false"); \
		return;                                                     \
	}

void printFile()
{
	auto f = SD.open("test.txt", FILE_WRITE);
	size_t fileSize = f.size();

	byte *buf = new byte[fileSize];

	f.seek(0);
	f.readBytes(buf, fileSize);
	f.close();

	for (int i = 0; i < fileSize; ++i) {
		Serial.print(byte_to_string(buf[i]));
	}
	Serial.println();

	delete[] buf;
}

void printRecord(uint32_t i, const char &c, int p)
{
	Serial.print("Record \'");
	Serial.print(c);
	Serial.print("\' #");
	Serial.print(i);
	Serial.print(" arg: ");
	Serial.println(p);
}

void printDatabase(Database<char, 8> &db)
{
	db.iterate(false, 0, db.size(), printRecord, 96);
}

void storeRecord(uint32_t i, const char &c, byte *buf)
{
	buf[i] = c;
}

bool checkRecords(Database<char, 8> &db)
{
	Serial.println("Checkrec");
	bool result = true;

	byte *buf1 = new byte[db.size()];
	auto size1 = db.size();
	db.iterate(false, 0, db.size(), storeRecord, buf1);

	db.trySave();

	byte *buf2 = new byte[db.size()];
	auto size2 = db.size();
	db.iterate(false, 0, db.size(), storeRecord, buf2);

	if (size2 != size1) {
		Serial.print("Neq size ");
		Serial.print(size1);
		Serial.print(" != ");
		Serial.println(size2);
		result = false;
	}
	else {
		for (size_t i = 0; i < size1; ++i) {
			if (buf1[i] != buf2[i]) {
				Serial.print("Mismatch at #");
				Serial.print(i);
				Serial.print(": ");
				Serial.print((char)buf1[i]);
				Serial.print(" != ");
				Serial.println((char)buf2[i]);
				result = false;
			}
		}
	}

	delete[] buf1;
	delete[] buf2;

	return result;
}

void testDatabase()
{
	Database<char, 8> testDB("test.txt");
	Serial.println(testDB.tryLoad());

	testDB.append('1');
	testDB.append('2');
	testDB.append('3');
	testDB.append('4');
	testDB.append('5');
	testDB.append('6');
	testDB.append('7');
	testDB.append('8');

	testDB.trySave();

	testDB.remove(7);
	testDB.append('9');
	REQUIRE(testDB.at(7).value == '9');

	testDB.modify(7, 'q');
	REQUIRE(testDB.at(7).value == 'q');
	testDB.modify(7, 'p');

	REQUIRE(testDB.at(7).value == 'p');
	REQUIRE(testDB.at(1).value == '2'); // --> '2'

	testDB.remove(1);
	REQUIRE(testDB.at(0).value == '1'); // --> '1'
	REQUIRE(testDB.at(1).value == '3'); // --> '3'

	REQUIRE(checkRecords(testDB));
	// printFile();

	REQUIRE(testDB.at(0).value == '1'); // --> '1'
	REQUIRE(testDB.at(1).value == '3'); // --> '3'

	testDB.remove(3);
	testDB.remove(4);
	REQUIRE(testDB.at(4).value == 'p'); // --> '8'
	testDB.modify(4, 'h');
	REQUIRE(testDB.at(4).value == 'h'); // --> 'h'

	REQUIRE(checkRecords(testDB));

	printDatabase(testDB);
	// printFile();
	REQUIRE(testDB.at(0).value == '1'); // --> '1'
	REQUIRE(testDB.at(1).value == '3'); // --> '3'
	REQUIRE(testDB.at(4).value == 'h'); // --> 'h'

	testDB.remove(0);
	testDB.remove(1);
	testDB.append('a');

	printDatabase(testDB);
	REQUIRE(testDB.at(4).state == QueryState::ERROR);
	REQUIRE(testDB.at(3).value == 'a');

	printDatabase(testDB);
	testDB.trySave();
	printDatabase(testDB);

	// REQUIRE(checkRecords(testDB));

	// printFile();
}

#endif // !defined(EKONYV_DATABASE_TEST_H)
