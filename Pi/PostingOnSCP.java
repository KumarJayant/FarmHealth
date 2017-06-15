import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.math.BigDecimal;
import java.math.RoundingMode;
import java.net.URL;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.List;
import java.util.Properties;
import java.util.Random;

import javax.net.ssl.HttpsURLConnection;

public class Temperature {

	// Set this to true if you are working behind a proxy server.
	// If you are working from home you should set this most probable to false
	public static boolean USEPROXY = true;
	// Add your account id here
	// Get the devide ID and the OAuth Token from the Device List in your
	// IoT Services Cockpit
	public static String MY_ACCOUNT_ID = "P1941838709trial";// "P1667685611trial";
	public static String MESSAGETYPE_ID = "d2d87e4e880fc87c8d69";// "fd5d652539f40e51e450";
	public static String DEVICE_1_ID = "58fb238c-cc8c-4587-b9d0-53dcb416c5c3";// "7c93e140-9703-40b6-bc4b-b639030ef631";
	public static String DEVICE_1_TOKEN = "b5357b2f62f611e7245a3a99ac3e8e";// "43c5b7eb66349b62b264a07dee652e79";

	// to be checked later
	public static String DEVICE_2_ID = "e9687a15-3305-4d6e-9e65-c821dd390340";
	public static String DEVICE_2_TOKEN = "80289b5afa8823093182d2d245b6139";

	public static void main(String args[]) {
		String repeatString = null;

		if (args.length > 0) {
			repeatString = args[0];
		} else {
			repeatString = "10";
		}
		int repeats = Integer.parseInt(repeatString);
		for (int i = 0; i < repeats; i++) {
			// send data every 2000ms
			try {
				Thread.sleep(2000);
			} catch (InterruptedException ie) {
				// Handle exception
			}
			// System.out.println("Loop " + (i + 1));
			long timestamp = System.currentTimeMillis();
			// double sensorTemperature = getOnboardTempSensor();
			double sensorTemperature = getRandomValue(-5, 5);
			String bodyMessage = buildBody(MESSAGETYPE_ID, sensorTemperature, timestamp);
			sendToCloud(bodyMessage, DEVICE_1_ID, DEVICE_1_TOKEN, String.valueOf(i + 1));
			sensorTemperature = getRandomValue(-5, 0);
			// sensorTemperature = getOnboardTempSensor();
			// sensorTemperature = getOneWireSensor("28-0000060a4638");
			// bodyMessage = buildBody(MESSAGETYPE_ID, sensorTemperature,
			// timestamp);
			// System.out.println(bodyMessage);
			// sendToCloud(bodyMessage, DEVICE_2_ID, DEVICE_2_TOKEN ,
			// String.valueOf(i + 1));
		}
	}

	// Creates random value within give limits
	private static double getRandomValue(double min, double max) {
		Random r = new Random();
		double value = min + (max - min) * r.nextDouble();

		// Round up the value to 2 decimal places
		// E.g. will make out of 1.42342232 a 1.42
		BigDecimal bd = new BigDecimal(value);
		bd = bd.setScale(2, RoundingMode.HALF_UP);
		return bd.doubleValue();

	}

	private static String buildBody(String messageType, Double sensorValue, long time) {
		String body = "{";
		body += buildJson("mode", "async") + ",";
		body += buildJson("messageType", messageType) + ",";
		body += '"' + "messages" + '"' + ":[";

		// timestamp - 1413191650
		// humidity
		// temperature
		// sunlight
		// nutrient

		String messageContent = "{";
		messageContent += buildJson("timestamp", 1413191650) + ",";
		messageContent += buildJson("humidity", sensorValue) + ",";
		messageContent += buildJson("temperature", sensorValue) + ",";
		messageContent += buildJson("sunlight", sensorValue) + ",";
		messageContent += buildJson("nutrient", time);
		messageContent += "}";

		body += messageContent;
		body += "]";

		body += "}";

		return body;
	}

	private static void sendToCloud(String body, String deviceId, String token, String loop) {

		// long sensorTimestamp = System.currentTimeMillis();
		String iotServiceMainLink = "https://iotmms" + MY_ACCOUNT_ID
				+ ".hanatrial.ondemand.com/com.sap.iotservices.mms/v1/api/http/data/";
		String url = iotServiceMainLink + deviceId;

		byte[] postData = body.getBytes();

		setProxy(USEPROXY);

		try {
			URL obj = new URL(url);
			String responseMessage = "<NONE>";
			// System.out.println(" Calling url " + url);

			HttpsURLConnection con = (HttpsURLConnection) obj.openConnection();
			con.setDoOutput(true);
			con.setInstanceFollowRedirects(false);
			// add request header
			con.setRequestMethod("POST");
			con.setRequestProperty("Content-Type", "application/json;charset=utf-8");
			con.setRequestProperty("Authorization", "Bearer " + token);

			// Send post request
			con.setDoOutput(true);
			DataOutputStream wr = new DataOutputStream(con.getOutputStream());
			wr.write(postData);
			wr.flush();
			wr.close();
			responseMessage = con.getResponseMessage();

			// Get the response from the server for the received data
			// InputStream in = con.getInputStream();
			// String encoding = con.getContentEncoding();
			// encoding = encoding == null ? "UTF-8" : encoding;

			System.out.println("  - Loop " + loop + " sent body: " + body);
			// System.out.println(" - Response from server : " +
			// IOUtils.toString(in, encoding));
			System.out.println("    - Response to connection : " + responseMessage);

		} catch (IOException e) {
			e.printStackTrace();
		}

	}

	private static void setProxy(boolean needsProxy) {

		Properties systemSettings = System.getProperties();
		if (needsProxy == true) {
			systemSettings.put("https.proxyHost", "proxy");
			systemSettings.put("https.proxyPort", "8080");
			systemSettings.put("http.proxyHost", "proxy");
			systemSettings.put("http.proxyPort", "8080");

		} else {
			systemSettings.put("http.proxySet", "false");
			systemSettings.put("https.proxySet", "false");
		}
	}

	private static String buildJson(String param, String value) {
		String result = "";
		char hk = '"';
		String paramPart = hk + param + hk;
		String valuePart = hk + value + hk;

		result = paramPart + ":" + valuePart;

		return result;
	}

	private static String buildJson(String param, long value) {
		String result = "";
		char hk = '"';
		String paramPart = hk + param + hk;
		String valuePart = String.valueOf(value);

		result = paramPart + ":" + valuePart;

		return result;
	}

	private static String buildJson(String param, double value) {
		String result = "";
		char hk = '"';
		String paramPart = hk + param + hk;
		String valuePart = String.valueOf(value);

		result = paramPart + ":" + valuePart;

		return result;
	}

	private static double getOnboardTempSensor() {

		String command = "/opt/vc/bin/vcgencmd measure_temp";

		String fileContent = getCommandOutput(command);
		String sensorValue = null;

		if (fileContent != null && fileContent.length() > 0) {
			String[] temp = fileContent.split("=");
			sensorValue = temp[1].trim();
			sensorValue = sensorValue.substring(0, sensorValue.length() - 2);
			System.out.println(" - Measured temperature for CPU sensor is " + sensorValue);
		}
		return Double.parseDouble(sensorValue);
	}

	private static double getOneWireSensor(String sensorHardwareId) {

		String sensorValue = null;
		String filename = "/sys/bus/w1/devices/" + sensorHardwareId + "/w1_slave";

		String fileContent = null;
		fileContent = readFile(filename);

		if (fileContent != null && fileContent.length() > 0) {
			String[] temp = fileContent.split("t=");
			sensorValue = temp[1].trim();
			System.out.println(" - Measured temperature for 1wire sensor " + sensorValue + " is " + sensorValue);
		}

		return Double.parseDouble(sensorValue);
	}

	private static String readFile(String filename) {
		String result = null;
		try {
			List<String> lines = Files.readAllLines(Paths.get(filename), Charset.defaultCharset());
			result = lines.get(1).toString();
		} catch (IOException e) {
			return null;
		}
		return result;
	}

	private static String getCommandOutput(String command) {

		String result = "";
		String s;
		Process p;

		try {
			p = Runtime.getRuntime().exec(command);
			BufferedReader br = new BufferedReader(new InputStreamReader(p.getInputStream()));
			while ((s = br.readLine()) != null)
				result += s;
			p.waitFor();
			p.destroy();
		} catch (Exception e) {
		}
		return result;
	}
}