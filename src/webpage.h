const char index_html[] PROGMEM = " \
<!DOCTYPE html> \
<html> \
	<head> \
		<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"> \
		<link rel=\"icon\" href=\"data:,\"> \
		<style> \
		html { \
			font-family: Helvetica; \
			display: inline-block; \
			margin: 0px auto; \
			text-align: center; \
		} \
		.button { \
			background-color: #4CAF50; \
			border: none; \
			color: white; \
			padding: 16px 40px; \
			text-decoration: none; \
			font-size: 30px; \
			margin: 2px; \
			cursor: pointer; \
		} \
		.button2 { \
			background-color: #555555; \
		} \
		</style> \
	</head> \
	\
	<body> \
		<h1>ESP32 Web Server</h1> \
		HAHA_TEST \
		<p><a href=\"/27/on\"><button class=\"button\">ON</button></a></p> \
		<p><a href=\"/27/off\"><button class=\"button button2\">OFF</button></a></p> \
	</body> \
</html> \
";