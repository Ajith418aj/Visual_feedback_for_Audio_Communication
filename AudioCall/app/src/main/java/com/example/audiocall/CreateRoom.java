package com.example.audiocall;

import static java.nio.ByteOrder.BIG_ENDIAN;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

import androidx.appcompat.app.AppCompatActivity;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.nio.ByteBuffer;

public class CreateRoom extends AppCompatActivity implements View.OnClickListener{

    private EditText room_id;
    private EditText password;

    private String button_clicked;

    private int audioPort;

    private int ackPort;

    private int rrPort;




    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_create_room);

        room_id = findViewById(R.id.room_id);
        password = findViewById(R.id.password);
        button_clicked = getIntent().getStringExtra("ButtonClicked");

        Button clicked_button;
        if(button_clicked.equals("C")) {
            clicked_button = findViewById(R.id.create_btn);
            clicked_button.setText("Create");
            clicked_button.setTag("Create");
            clicked_button.setOnClickListener(this);

        } else if (button_clicked.equals("J")) {
            clicked_button = findViewById(R.id.create_btn);
            clicked_button.setText("Join");
            clicked_button.setTag("Join");
            clicked_button.setOnClickListener(this);
        }


    }

    Socket socket = null;
    DataOutputStream dataOutputStreamInstance = null;

    String roomName = null;

    public void send(byte[] data) {
        Log.d("CreateRoom", "Sending data...");
        try {
            dataOutputStreamInstance = new DataOutputStream(socket.getOutputStream());
        } catch (IOException e) {
            Log.e("CreateRoom", "Error creating DataOutputStream: " + e.getMessage());
            throw new RuntimeException(e);
        }
        try {
            dataOutputStreamInstance.write(data);
            dataOutputStreamInstance.flush();
            Log.d("CreateRoom", "Data sent successfully.");
        } catch (IOException e) {
            Log.e("CreateRoom", "Error sending data: " + e.getMessage());
            throw new RuntimeException(e);
        }
    }

    //public static final String IP_ADDRESS = "10.96.23.36";
    public static final String IP_ADDRESS = "10.129.131.181";
    //public static final int PORT_NUMBER = 8081;
    public static final int PORT_NUMBER = 8081;

    @Override
    public void onClick(View view) {
        roomName = room_id.getText().toString();
        String roomPass = password.getText().toString();
        String request = view.getTag().toString();
        Intent intent = new Intent(this, Call.class);

        Thread thread = new Thread(new Runnable() {
            @Override
            public void run() {
                try {

                    //socket = new Socket("10.129.131.231", 8086);
                    socket = new Socket(IP_ADDRESS, PORT_NUMBER);
                    String str = String.join(" ", request, roomName, roomPass);
                    byte[] request_bytes = str.getBytes();
                    byte[] len = ByteBuffer.allocate(4).order(BIG_ENDIAN).putInt(str.length()).array();
                    byte[] data = new byte[4+request_bytes.length];
                    System.arraycopy(len, 0, data, 0, 4);
                    System.arraycopy(request_bytes, 0, data, 4, request_bytes.length);

//                    dataOutputStreamInstance = new DataOutputStream(socket.getOutputStream());
//                    dataOutputStreamInstance.write(data);
//                    dataOutputStreamInstance.flush();
                    send(data);

                    /* Receive TCP, UDP port details from control.py
                        TCP -  for acknowledgement and reception report
                        UDP - for sending audio data.
                     */
                    DataInputStream dataInputStreamInstance = new DataInputStream(socket.getInputStream());

                    byte[] ports = new byte[1024];

                    dataInputStreamInstance.read(ports);

                    byte[] portRRBytes = new byte[4];
                    System.arraycopy(ports, 4, portRRBytes, 0, 4);
                    audioPort = ByteBuffer.wrap(portRRBytes).getInt();

                    byte[] audioPortBytes = new byte[4];
                    System.arraycopy(ports, 8, audioPortBytes, 0, 4);
                    ackPort = ByteBuffer.wrap(audioPortBytes).getInt();

                    byte[] ackPortBytes = new byte[4];
                    System.arraycopy(ports, 12, ackPortBytes, 0, 4);
                    rrPort = ByteBuffer.wrap(ackPortBytes).getInt();

                    Log.d("Received Ports", "Audio Port = "+ audioPort +
                            " Ack Port = "+ackPort + "RR Port = "+ rrPort);


                    intent.putExtra("audioPort", audioPort);
                    intent.putExtra("ackPort", ackPort);
                    intent.putExtra("rrPort", rrPort);
                    intent.putExtra("room_name", roomName);

                    //startActivity(intent);
                    startActivityForResult(intent, 1001);
                } catch (IOException e) {
                    throw new RuntimeException(e);
                }
            }
        });
        thread.start();


    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data2) {
        super.onActivityResult(requestCode, resultCode, data2);

        if (requestCode == 1001) {
            if (resultCode == RESULT_OK) {
                // This code will execute when the called activity returns successfully
                String str = String.join(" ", "Endcall", roomName);
                byte[] request_bytes = str.getBytes();
                byte[] len = ByteBuffer.allocate(4).order(BIG_ENDIAN).putInt(str.length()).array();
                byte[] data = new byte[4+request_bytes.length];
                System.arraycopy(len, 0, data, 0, 4);
                System.arraycopy(request_bytes, 0, data, 4, request_bytes.length);



                Thread end = new Thread(new Runnable() {
                    @Override
                    public void run() {
                        //                            dataOutputStreamInstance.write(data);
//                            dataOutputStreamInstance.flush();
                        send(data);
                        Log.d("CreateRoom", "Activity Over1");
                    }
                });
                end.start();
                Log.d("CreateRoom", "Activity Over2");
            }
        }
    }

}