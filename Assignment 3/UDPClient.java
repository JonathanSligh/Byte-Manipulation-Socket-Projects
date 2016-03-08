import java.io.*;
import java.net.*;

class UDPClient {
	public static void main(String args[]) throws Exception {
		DatagramSocket clientSocket = new DatagramSocket();
		InetAddress IPAddress =
		InetAddress.getByName(args[0]);
		byte[] sendData = new byte[1024];
		byte[] receiveData = new byte[1024];
		short specialMessage = (short) 42505;
		sendData[0] = (byte) ((specialMessage >> 8) & 0xFF);
		sendData[1] = (byte) (specialMessage & 0xFF);
		short port = Short.parseShort(args[2]);
		sendData[2] = (byte) ((port >> 8) & 0xFF);
		sendData[3] = (byte) (port & 0xFF);
		sendData[4] = 0x07;
		if (Integer.parseInt(args[2]) < 10045 || Integer.parseInt(args[2]) > 10049) {
			System.exit(0);
		}
		DatagramPacket sendPacket =
		new DatagramPacket(sendData, 5,
		IPAddress, Integer.parseInt(args[1]));
		clientSocket.send(sendPacket);
		DatagramPacket receivePacket =
		new DatagramPacket(receiveData,
		receiveData.length);
		clientSocket.receive(receivePacket);
		byte[] received = receivePacket.getData();
		System.out.println(received.length);
		String a5 = "A5A5";
		if (received[5] == 0) {
			short r = (short) ((short) (received[3] << 8) | received[4]);
			if (r < 8 && r > 0) {
				System.out.println("ERROR: incorrect request");
				System.exit(0);
			}
			System.out.println("FROM SERVER:" +
			a5 + (int) received[2] + r);
			ServerSocket serverSocket = null;

			try {
				serverSocket = new ServerSocket(Integer.parseInt(args[2]));
			} catch (IOException e) {
				System.err.println("Could not listen on port: 10007.");
				System.exit(1);
			}

			Socket clientSocket2 = null;
			System.out.println("Waiting for a partner to connect...");

			try {
				clientSocket2 = serverSocket.accept();
			} catch (IOException e) {
				System.err.println("Accept failed.");
				System.exit(1);
			}

			System.out.println("Connection successful");
			System.out.println("Waiting for input.....");

			PrintWriter out = new PrintWriter(clientSocket2.getOutputStream(),
			true);
			BufferedReader in = new BufferedReader(
			new InputStreamReader(clientSocket2.getInputStream()));
			BufferedReader stdIn = new BufferedReader(
			new InputStreamReader(System.in));
			String inputLine;
			String userInput;
			out.println("Please Enter a Message");
			while (true) {
				System.out.println((inputLine = in.readLine()));
				if (inputLine.toLowerCase().equalsIgnoreCase("Bye Bye Birdie"))
				break;
				userInput = stdIn.readLine(); 
				out.println(userInput);
				out.flush();
				if (userInput.toLowerCase().equalsIgnoreCase("Bye Bye Birdie"))
				break;
			}

			out.close();
			in.close();
			clientSocket2.close();
			serverSocket.close();

		} else {
			short port1 = (short) ((short) (received[16] << 8) | received[17]);
			String serverHostname = "";
			for (int i = 2; i < 16; i++) {
				serverHostname += (char) received[i];
			}
			System.out.println("Attemping to connect to host " +
			serverHostname + " on port " + port1);
			Socket echoSocket = null;
			PrintWriter out = null;
			BufferedReader in = null;

			try {
				echoSocket = new Socket(serverHostname, port1);
				out = new PrintWriter(echoSocket.getOutputStream(), true);
				in = new BufferedReader(new InputStreamReader(
				echoSocket.getInputStream()));
			} catch (UnknownHostException e) {
				System.err.println("Don't know about host: " + serverHostname);
				System.exit(1);
			} catch (IOException e) {
				System.err.println("Couldn't get I/O for "
				+ "the connection to: " + serverHostname);
				System.exit(1);
			}

			BufferedReader stdIn = new BufferedReader(
			new InputStreamReader(System.in));
			String userInput;
			String inputLine;
			while (true) {
				System.out.println((inputLine = in.readLine()));
				if (inputLine.toLowerCase().equalsIgnoreCase("bye bye birdie"))
				{
					out.close();
					in.close();
					stdIn.close();
					echoSocket.close();
					break;
				}
				userInput = stdIn.readLine();
				out.println(userInput);
				out.flush();
				if (userInput.toLowerCase().equalsIgnoreCase("bye bye birdie")) {
					out.close();
					in.close();
					stdIn.close();
					echoSocket.close();
					break;
				}

			}

		}

		clientSocket.close();
		
	}
}
