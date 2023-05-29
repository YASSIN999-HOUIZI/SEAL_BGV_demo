package com.example.seal_bgv_demo;

import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

import com.example.seal_bgv_demo.R;

import android.content.DialogInterface;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.EditText;

import java.io.File;
import java.io.IOException;
import java.util.concurrent.CountDownLatch;

public class MainActivity extends AppCompatActivity {
    static {
        System.loadLibrary("sealmain");
    }
    EditText input1;
    EditText input2;

    public static File path;
    public static String Pk_Path;
    public static String CloudData1_Path;
    public static String CloudData2_Path;
    public static String SecretKey_Path;
    private final String cppVerifierUrl = "192.168.1.7:8040";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        path = getFilesDir();
        Pk_Path = path+"/PK.key";
        CloudData1_Path = path+"/CD1.data";
        CloudData2_Path = path+"/CD2.data";
        SecretKey_Path = path+"/SK.key";
        input1 = findViewById(R.id.editTextNumber1);
        input2 = findViewById(R.id.editTextNumber2);
    }
    public native int Decrypt(String ClaimPath, String SK_Path);
    public native int init(String SecretKey_Path,String Pk_Path,String CloudData1_Path,String CloudData2_Path, int x, int y);
    public void Click(View view) throws IOException, InterruptedException {

        long begin = System.currentTimeMillis();
        String path = String.valueOf(MainActivity.path);
        int x = Integer.parseInt(input1.getText().toString());
        int y = Integer.parseInt(input2.getText().toString());

        int status = init(SecretKey_Path,Pk_Path,CloudData1_Path,CloudData2_Path,x,y);
        System.out.println("status:"+status);
        ClientEndpoint proofEndpoint = new ClientEndpoint();
        proofEndpoint.createWebSocketClient("ws://" + cppVerifierUrl);
        //proofEndpoint.latch.await();
        proofEndpoint.webSocketClient.connect();
        proofEndpoint.latch.await();
        proofEndpoint.latch = new CountDownLatch(1);
        proofEndpoint.webSocketClient.send("SEAL");
        proofEndpoint.sendFile(path + "/CD1.data", "CD1.data");
        proofEndpoint.sendFile(path + "/CD2.data", "CD2.data");
        proofEndpoint.sendFile(path + "/PK.key", "PK.key");

        proofEndpoint.latch.await();
        int r = Decrypt(path+"/Answer.data", path+"/SK.key");
        System.out.println("r: "+r);

        proofEndpoint.webSocketClient.close();
        // Create and configure the AlertDialog
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Multiplication Result");
        builder.setMessage("Result: " + r);
        builder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                // User clicked OK, do something if needed
            }
        });

// Show the AlertDialog
        AlertDialog dialog = builder.create();
        dialog.show();
        //End time
        long end = System.currentTimeMillis();

        long time = end-begin;
        System.out.println();
        System.out.println("Elapsed Time: "+time +" milli seconds");
        Log.d("Result of the multiplication :", String.valueOf(r));

    }
}