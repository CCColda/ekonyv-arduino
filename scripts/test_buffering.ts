import { readRecord } from "https://deno.land/std@0.153.0/encoding/csv/_io.ts";

type Action<T> = {
	type: 'append',
	record: T
} | {
	type: 'modify',
	n: number,
	record: T
} | {
	type: 'remove',
	n: number
};

class BufferedData<T> {
	private data: T[] = [];
	private data_instant: T[] = [];
	private actions: Action<T>[] = [];
	private numRecords = 0;
	private maxActions = 8;

	private executeOnData(action: Action<T>, dataRef: T[]) {
		switch (action.type) {
			case 'append': {
				dataRef.push(action.record);
				break;
			}
			case 'modify': {
				dataRef[action.n] = action.record;
				break;
			}
			case 'remove': {
				dataRef.splice(action.n, 1);
				break;
			}
		}
	}

	append(data: T) {
		this.actions.push({
			type: 'append',
			record: data
		});

		++this.numRecords;

		this.executeOnData(this.actions[this.actions.length - 1], this.data_instant);

		return this.actions.length != this.maxActions;
	}

	modify(n: number, newData: T) {
		this.actions.push({
			type: 'modify',
			n,
			record: newData
		});

		this.executeOnData(this.actions[this.actions.length - 1], this.data_instant);

		return this.actions.length != this.maxActions;
	}

	private tryUpdateBufferWithRemove(n: number) {
		let fileItems = this.data.length;
		let app_removed = false;
		let k = this.actions.length;
		for (let i = 0; i < k; i++) {
			const update = this.actions[i];

			switch (update.type) {
				case 'append': {
					if (fileItems == n) {
						this.actions.splice(i, 1);
						app_removed = true;
						k--;
						i--;
					}

					fileItems++;
					break;
				}
				case 'modify': {
					if (app_removed) {
						if (update.n == n) {
							this.actions.splice(i, 1);
							k--;
							i--;
						}
						else if (update.n > n) {
							update.n--;
						}
					}

					break;
				}
				case 'remove': {
					fileItems--;
					break;
				}
			}
		}

		return app_removed;
	}

	remove(n: number) {
		if (n < 0 || n >= this.numRecords)
			return false;

		if (!this.tryUpdateBufferWithRemove(n)) {
			this.actions.push({
				type: 'remove',
				n
			});
		}

		this.executeOnData({
			type: 'remove',
			n
		}, this.data_instant);

		--this.numRecords;

		return this.actions.length != this.maxActions;
	}

	instantAt(n: number) {
		return this.data_instant[n];
	}

	at(n: number) {
		if (n >= this.numRecords) {
			return null;
		}

		let fileIdx = n,
			fnumRecords = this.data.length,
			data: T | null = null;

		for (const update of this.actions) {
			switch (update.type) {
				case 'remove': {
					if (update.n <= n) {
						fileIdx++;
						data = null;
					}

					fnumRecords--;
					break;
				}
				case 'modify': {
					if (update.n == n) {
						data = update.record;
					}

					break;
				}
				case 'append': {
					if (fnumRecords == n) {
						data = update.record;
					}

					fnumRecords++;
					break;
				}
			}
		}

		if (data !== null) {
			//console.log(`Found data ${n} in cache: ${data}`);
			return data;
		}
		else {
			//console.log(`${n} not in cache: ${this.data[fileIdx]} | ${this.data_instant[n]}`);
			return this.data[fileIdx];
		}
	}

	flush() {
		let item: Action<T>;

		while (([item] = this.actions.splice(0, 1)) && item) {
			this.executeOnData(item, this.data);
		}
	}

	debug() {
		console.log(`BDATA: ${JSON.stringify(this.data)}\nIDATA: ${JSON.stringify(this.data_instant)}\nACT:   ${JSON.stringify(this.actions)}`);
	}

	get size() {
		return this.numRecords;
	}
}

if (import.meta.main) {
	const data = new BufferedData<number>();

	for (let i = 0; i < 10000000; i++) {
		const n = Math.floor(Math.random() * 5);

		if (n == 0 && data.size > 0) {
			console.log("MODIFY");
			data.modify(data.size - 1, Math.floor(Math.random() * 100));
		}
		else if ((n == 1 && data.size > 0)) {
			const item = Math.floor(Math.random() * (data.size - 1));
			console.log("REMOVE " + item);
			data.remove(item);
		}
		else {
			console.log("APPEND");
			data.append(Math.floor(Math.random() * 100));
		}

		const records = Array(data.size).fill(0)
			.map((_, i) => [data.at(i), data.instantAt(i)]);

		const success = records.map(([a, b]) => a == b).reduce((a, b) => a && b, true);

		if (!success) {
			console.log("FAIL");
			data.at(data.size - 1);
			console.log(JSON.stringify(records));
			data.debug();
			break;
		}
	}
}