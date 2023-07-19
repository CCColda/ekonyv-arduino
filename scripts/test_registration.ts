#! deno

import { parse } from "https://deno.land/std@0.153.0/encoding/csv.ts";
import * as flags from "https://deno.land/std@0.192.0/flags/mod.ts";
import * as path from "https://deno.land/std@0.192.0/path/mod.ts";

const TEST_CREDS = {
	username: "TestRegistration",
	password: "no"
};

type BookData = {
	id?: number,
	in: number,
	title: string,
	authors: string,
	attributes: string,
	published: string,
	created?: number,
	storage_id: number,
	user_id?: number,
	flags: string
};

const books: BookData[] = [{
	id: 0,
	in: 9789634970323,
	title: "A teremtők árnyai / Gabriel Mesta ; [ford. Szente Mihály].",
	authors: "Mesta, Gabriel.",
	created: 0,
	storage_id: 1,
	user_id: 0,
	attributes: "",
	published: "",
	flags: "wrb"
}, {
	id: 0,
	in: 9789633245187,
	title: "Legendás állatok és megfigyelésük / J. K. Rowling, Göthe Salmander ; [ford. Tóth Tamás Boldizsár].",
	authors: "Rowling, J. K. (1965-).",
	attributes: "138, [6] p. : ill. ; 21 cm",
	published: "[Budapest] : Animus, 2021.",
	created: 0,
	storage_id: 1,
	user_id: 0,
	flags: "wrb"
}];

async function requestCode(ip: string): Promise<string | null> {
	const resp = await fetch(`http://${ip}:80/api/user/req_code`, { method: 'POST' });

	if (resp.status != 200)
		return null;

	const response_text = await resp.text();

	const csv_data = parse(response_text, { fieldsPerRecord: 2 });
	const code = csv_data.find(([k, v]) => k == "code")?.[1];

	console.log(response_text);
	console.log(csv_data);

	return code || null;
}

async function requestRegistration(ip: string, code: string): Promise<boolean> {
	const resp = await fetch(`http://${ip}:80/api/user/register?code=${code}&username=${TEST_CREDS.username}&password=${TEST_CREDS.password}`, { method: 'POST' });

	if (resp.status != 200)
		return false;

	const response_text = await resp.text();

	const csv_data = parse(response_text, { fieldsPerRecord: 2 });

	console.log(response_text);
	console.log(csv_data);

	const state = csv_data.find(([k, v]) => k == "state")?.[1];

	return (state ?? "") == "success";
}

async function requestSession(ip: string): Promise<string | null> {
	const resp = await fetch(`http://${ip}:80/api/user/login?username=${TEST_CREDS.username}&password=${TEST_CREDS.password}`, { method: 'POST' });

	if (resp.status != 200)
		return null;

	const response_text = await resp.text();

	const csv_data = parse(response_text, { fieldsPerRecord: 2 });

	console.log(response_text);
	console.log(csv_data);

	const token = csv_data.find(([k, v]) => k == "token")?.[1];

	return token ?? null;
}

async function requestPrivateHelloPage(ip: string, token: string): Promise<boolean> {
	const resp = await fetch(`http://${ip}:80/api/user/hello?token=${token}`, { method: 'GET' });

	if (resp.status != 200)
		return false;

	const response_text = await resp.text();

	console.log(response_text);
	return true;
}

async function addBook(ip: string, token: string, book_data: BookData): Promise<boolean> {
	const book_data_query = Object.entries(book_data).map(([k, v]) => `${k}=${v}`).join('&');

	const resp = await fetch(`http://${ip}:80/api/book?token=${token}&${book_data_query}`, { method: 'POST' });

	if (resp.status != 200)
		return false;

	const response_text = await resp.text();

	console.log(response_text);
	return true;
}

async function getAllBooks(ip: string, token: string) {
	const resp = await fetch(`http://${ip}:80/api/book/all?token=${token}`, { method: 'GET' });

	if (resp.status != 200)
		return false;

	const response_text = await resp.text();
	console.log(response_text);

	const csv_data = parse(response_text);

	console.log(csv_data);
	return true;
}

async function getBookCount(ip: string, token: string) {
	const resp = await fetch(`http://${ip}:80/api/book/count?token=${token}`, { method: 'GET' });

	if (resp.status != 200)
		return false;

	const response_text = await resp.text();

	const csv_data = parse(response_text, { fieldsPerRecord: 2 });

	console.log(response_text);
	console.log(csv_data);
	return true;
}

async function main() {
	const args = flags.parse(Deno.args);
	const ip = args["ip"] ?? null;
	const code = args["code"] ?? null;

	if (!ip) {
		console.error(
			"Usage: deno run " + path.fromFileUrl(import.meta.url) + " <--ip=xxx.xxx.xxx.xxx> [--code=xxxx]"
		);
		return;
	}

	const reqcode = code ?? await requestCode(ip);
	if (!reqcode) {
		console.error("Failed getting code");
		return;
	}
	console.log("Getting code successful");

	if (!await requestRegistration(ip, reqcode)) {
		console.log("Failed registration");
		return;
	}
	console.log("Registration successful");

	const session = await requestSession(ip);

	if (!session) {
		console.error("Failed getting session token");
		return;
	}
	console.log("Successfully got session token");

	for (const book of books) {
		if (!await addBook(ip, session, book)) {
			console.error("Failed adding book");
			return;
		}

		console.log("Successfully added book");
	}

	if (!await getBookCount(ip, session)) {
		console.error("Failed getting book count");
		return;
	}

	console.log("Successfully got book count");

	if (!await getAllBooks(ip, session)) {
		console.error("Failed getting books");
		return;
	}

	console.log("Successfully got all books");


	/* if (!await requestPrivateHelloPage(ip, session)) {
		console.error("Failed getting private page");
		return;
	}

	console.log("Getting private page successful"); */
}


if (import.meta.main) {
	await main();
}
