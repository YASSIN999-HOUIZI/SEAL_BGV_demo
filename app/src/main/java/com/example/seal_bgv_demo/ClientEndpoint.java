package com.example.seal_bgv_demo;

import com.google.gson.Gson;

import org.apache.commons.io.FileUtils;
import org.java_websocket.client.WebSocketClient;
import org.java_websocket.handshake.ServerHandshake;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import javax.websocket.OnMessage;


public class ClientEndpoint {
    public String response;
    public CountDownLatch latch = new CountDownLatch(1);
    public WebSocketClient webSocketClient;
    private File fileOutput;
    List<Byte> fileByte=new ArrayList<Byte>();
    private Gson gson = new Gson();
    public int pos;

    public WebSocketClient createWebSocketClient(String url){
        URI uri;
        try {
            uri = new URI(url);
        }
        catch (URISyntaxException e) {
            e.printStackTrace();
            return null;
        }

        webSocketClient = new WebSocketClient(uri) {
            @Override
            public void onOpen(ServerHandshake handshakedata) {
                System.out.println("onOpen");
                //webSocketClient.send(response);
                latch.countDown();
                //latch = new CountDownLatch(1);
            }

            private String fileName; // Declare a member variable to store the file name
            private FileOutputStream fos; // Declare a member variable for file output stream

// ...

            @OnMessage(maxMessageSize = 15000000)
            public void onMessage(ByteBuffer buffer) {
                try {
                    // Check if it's the first chunk of the file (name)
                    if (fileName == null) {
                        // Read the file name from the buffer
                        byte[] fileNameBytes = new byte[buffer.remaining()];
                        buffer.get(fileNameBytes);
                        fileName = new String(fileNameBytes);
                        System.out.println("File name received: " + fileName);

                        // Create a new file output stream
                        File file = new File(MainActivity.path + "/" + fileName);
                        fos = new FileOutputStream(file);
                    } else {
                        // Read the byte data from the buffer
                        byte[] chunk = new byte[buffer.remaining()];
                        buffer.get(chunk);
                        // Write the chunk to the file
                        fos.write(chunk);
                        fos.flush();

                        System.out.println("Chunk of binary data received and appended to the file");
                    }
                } catch (IOException e) {
                    throw new RuntimeException(e);
                }
                //latch.countDown();
            }



            @Override
            public void onMessage(String message) {
                System.out.println("this is the response :"+message);
                response = message;
                if (message.equals("DONE")) {
                    if (latch != null) {
                        latch.countDown();
                    }
                }
                /*try {
                    TimeUnit.SECONDS.sleep(2);
                } catch (InterruptedException e) {
                    throw new RuntimeException(e);
                }
                latch.countDown();*/

            }
            @Override
            public void onError(Exception ex) {
                ex.printStackTrace();
            }

            @Override
            public void onClose(int code, String reason, boolean remote) {
                System.out.println("onCloseReceived: " + code + ", " + reason);
            }
        };
        return webSocketClient;
    }
    public void sendFile(String filePath, String name) throws IOException {

        File file = new File(filePath);
        byte[] bytes = FileUtils.readFileToByteArray(file);
        int from, to;
        from = 0;
        to = 8192;
        while (bytes.length > to){
            webSocketClient.send(Arrays.copyOfRange(bytes, from, to));
            from = to;
            to += 8192;
        }
        webSocketClient.send(Arrays.copyOfRange(bytes, from, bytes.length));
        webSocketClient.send(name);
    }
}

