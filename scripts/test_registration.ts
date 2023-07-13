#! deno

import { parse } from "https://deno.land/std@0.153.0/encoding/csv.ts";
import * as flags from "https://deno.land/std@0.192.0/flags/mod.ts";
import * as path from "https://deno.land/std@0.192.0/path/mod.ts";

const TEST_CREDS = {
	username: "TestRegistration",
	password: "no"
};

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

	if (!await requestPrivateHelloPage(ip, session)) {
		console.error("Failed getting private page");
		return;
	}

	console.log("Getting private page successful");
}


if (import.meta.main) {
	await main();
}
