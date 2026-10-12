#pragma once

enum HttpStatusCode
{
	OK = 200,
	MovedPermanently = 301,
	Found = 302,
	TemporaryRedirect = 307,
	PermanentRedirect = 308,
	BadRequest = 400,
	Forbidden = 403,
	FileNotFound = 404,
	MethodNotAllowed = 405,
	UnsupportedMediaType = 415,
	InternalServerError = 500,
};
