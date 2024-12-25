#ifndef EKONYV_CHANGE_DB_H
#define EKONYV_CHANGE_DB_H

#include "../storage/database.h"
#include "../types/callback.h"
#include "search.h"

struct Change {
	unsigned long timestamp;
	
	struct {
		uint8_t database_id : 6;
		BlockfileUpdateType update_type : 2;
	};

	uint32_t id;
};

class ChangeDatabase {
public:
	Database<Change, EK_CHANGEDB_BUFFER> db;

public:
	ChangeDatabase();
	void load();
	void save();

	void storeChange(uint8_t db_id, BlockfileUpdateType update_type, uint32_t id);
};

#endif // !defined(EKONYV_CHANGE_DB_H)