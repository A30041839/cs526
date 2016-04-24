import java.net.*;
import java.io.*;
import org.json.simple.*;

public class TestDecentralized {

	public static void simpleTest() throws Exception {
		int numberOfNodes = 20;
		for (int i = 0; i < numberOfNodes; i++) {
			sendRequest("add_node", new String[] { String.valueOf(i) });
		}

		for (int i = 0; i < numberOfNodes; i++) {
			for (int j = i + 1; j < numberOfNodes; j++) {
				sendRequest("add_edge", new String[] { String.valueOf(i), String.valueOf(j) });
			}
		}

		for (int i = 0; i < numberOfNodes / 2; i++) {
			sendRequest("remove_node", new String[] { String.valueOf(i) });
		}

		for (int i = numberOfNodes / 2 + 1; i < numberOfNodes; i++) {
			sendRequest("remove_edge", new String[] { String.valueOf(i - 1), String.valueOf(i) });
		}
	}

	public static void main(String[] args) throws Exception {
		simpleTest();
	}

	@SuppressWarnings("unchecked")
	public static void sendRequest(String method, String[] nodes) throws Exception {
		URL url = new URL("http://40.117.95.162:8000/api/v1/" + method);
		HttpURLConnection connection = (HttpURLConnection) url.openConnection();

		connection.setRequestMethod("POST");
		connection.setDoOutput(true);
		// connection.addRequestProperty("Content-Type",
		// "application/x-www-form-urlencoded");

		JSONObject data = new JSONObject();
		if (nodes.length == 0) {

		} else if (nodes.length == 1) {
			data.put("node_id", nodes[0]);
		} else {
			data.put("node_a_id", nodes[0]);
			data.put("node_b_id", nodes[1]);
		}

		OutputStreamWriter writer = new OutputStreamWriter(connection.getOutputStream());
		writer.write(data.toJSONString());

		writer.close();

		System.out.println(connection.getResponseCode());
		if (connection.getResponseCode() == HttpURLConnection.HTTP_OK) {
			BufferedReader reader = new BufferedReader(new InputStreamReader(connection.getInputStream(), "UTF-8"));
			String result = "";
			String line;
			while ((line = reader.readLine()) != null) {
				result += line;
			}
			System.out.println(result);
		}
	}
}
