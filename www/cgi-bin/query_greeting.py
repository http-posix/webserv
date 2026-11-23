import html
import os
import random
import sys
from urllib.parse import parse_qs

GREETINGS = {
	"en": "Hello, {}!",
	"nl": "Hallo, {}!",
	"uk": "Привіт, {}!",
	"fr": "Bonjour, {} !",
}

PHRASES = {
	"en": [
		"Nice to see you here.",
		"Have a great day!",
		"May your code compile on the first try.",
	],
	"nl": [
		"Leuk je te zien.",
		"Fijne dag nog!",
		"Moge je code in één keer compileren.",
	],
	"uk": [
		"Раді тебе бачити.",
		"Гарного дня!",
		"Хай код компілюється з першого разу.",
	],
	"fr": [
		"Ravi de te voir ici.",
		"Bonne journée !",
		"Que ton code compile du premier coup !",
	],
}

DEFAULT_LANG = "en"
DEFAULT_NAME = "stranger"


def get_param(params, key, default):
	values = params.get(key)
	if not values or not values[0].strip():
		return default
	return values[0].strip()


def main():
	params = parse_qs(os.environ.get("QUERY_STRING", ""))
	name = get_param(params, "name", DEFAULT_NAME)
	lang = get_param(params, "lang", DEFAULT_LANG).lower()
	if lang not in GREETINGS:
		lang = DEFAULT_LANG

	greeting = GREETINGS[lang].format(html.escape(name))
	phrase = random.choice(PHRASES[lang])

	sys.stdout.write("Content-Type: text/html; charset=utf-8\r\n\r\n")
	sys.stdout.write(
		f"<html><body><h1>{greeting}</h1><p>{phrase}</p></body></html>\n"
	)


if __name__ == "__main__":
	main()