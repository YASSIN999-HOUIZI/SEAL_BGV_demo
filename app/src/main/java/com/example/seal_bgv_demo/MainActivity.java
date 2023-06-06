package com.example.seal_bgv_demo;

import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

import android.content.DialogInterface;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.EditText;

import java.io.File;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.ArrayList;
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
    public static String CloudData3_Path;
    public static String SecretKey_Path;
    private final String cppVerifierUrl = "192.168.179.36:8050";
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        path = getFilesDir();
        input1 = findViewById(R.id.editTextNumber1);
        input2 = findViewById(R.id.editTextNumber2);
    }
    public native int[] Decrypt(String ClaimPath, String SK_Path, int bytes);
    public native int[] vote_Decrypt(String ClaimPath, String SK_Path);
    public native int init(String SecretKey_Path,String Pk_Path,String CloudData1_Path,String type, int x,int bytes);
    public native int init_vote(String SecretKey_Path,String Pk_Path,String CloudData1_Path, int x, int y,int z);
    public void Click(View view) throws IOException, InterruptedException {

        long begin = System.currentTimeMillis();
        String path = String.valueOf(MainActivity.path);
        int x = Integer.parseInt(input1.getText().toString());
        String claim = input2.getText().toString();
        System.out.println("Verification for : "+claim);
        Pk_Path = path+"/"+claim+"Cloud.key";
        SecretKey_Path = path+"/"+claim+"SK.key";
        CloudData1_Path = path+"/"+claim+"Cloud.data";
        CloudData2_Path = path+"/"+claim+"CD.data";
        int status;
        if(claim.equals("sin")){
            status = init(SecretKey_Path,Pk_Path,CloudData1_Path,claim,x,40);
        }else {
            status = init(SecretKey_Path,Pk_Path,CloudData1_Path,claim,x,20);
        }
        //int status = init_vote(SecretKey_Path,Pk_Path,CloudData1_Path,CloudData2_Path,CloudData3_Path, x, y,z1);
        System.out.println("status:"+status);
        ClientEndpoint proofEndpoint = new ClientEndpoint();
        proofEndpoint.createWebSocketClient("ws://" + cppVerifierUrl);
        //proofEndpoint.latch.await();
        proofEndpoint.webSocketClient.connect();
        proofEndpoint.latch.await();
        proofEndpoint.latch = new CountDownLatch(1);
        proofEndpoint.pos = 0;
        proofEndpoint.webSocketClient.send(claim);
        //proofEndpoint.sendFile(path + "/CD1.data", "CD1.data");
        //proofEndpoint.sendFile(path + "/CD2.data", "CD2.data");
        proofEndpoint.sendFile(path + "/"+claim+"Cloud.key", claim+"Cloud.key");
        proofEndpoint.sendFile(path + "/"+claim+"Cloud.data", claim+"Cloud.data");
        System.out.println("result");
        proofEndpoint.latch.await();
        int proof[];
        if(claim.equals("sin"))
        {
            proof = Decrypt(path + "/Answer.data", path + "/" + claim + "SK.key",40);
        }
        else
        {
            proof = Decrypt(path + "/Answer.data", path + "/" + claim + "SK.key", 20);
        }

        System.out.println("proof array size :"+proof.length);
        for (int k : proof) {
            System.out.println("result :" + k);
        }

        String num = String.valueOf(16);
        proofEndpoint.webSocketClient.send(num);
        for (int j : proof) {
            num = String.valueOf(j);
            proofEndpoint.webSocketClient.send(num);
        }

        proofEndpoint.latch.await();
        proofEndpoint.webSocketClient.close();
        // Create and configure the AlertDialog
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Verification");
        builder.setMessage("finish");
        builder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                // User clicked OK, do something if needed
            }
        });
        AlertDialog dialog = builder.create();
        dialog.show();
        //End time
        long end = System.currentTimeMillis();
        long time = end-begin;
        System.out.println();
        System.out.println("Elapsed Time: "+time +" milli seconds");
        Log.d("Result of the multiplication :", String.valueOf(0));

    }
    public void VoteClick(View view) throws IOException, InterruptedException {

        long begin = System.currentTimeMillis();
        String path = String.valueOf(MainActivity.path);
        String claim = "vote";
        System.out.println("Verification for : "+claim);
        Pk_Path = path+"/"+claim+"Cloud.key";
        SecretKey_Path = path+"/"+claim+"SK.key";
        CloudData1_Path = path+"/"+claim+"Cloud.data";
        CloudData2_Path = path+"/"+claim+"CD.data";
        int status;
        int x=2,y=4,z=2;
        int A = (x*x*x)+(y*y*y)+(z*z*z);
        status = init_vote(SecretKey_Path,Pk_Path,CloudData1_Path,x,y,z);
        //int status = init_vote(SecretKey_Path,Pk_Path,CloudData1_Path,CloudData2_Path,CloudData3_Path, x, y,z1);
        System.out.println("status:"+status);
        ClientEndpoint proofEndpoint = new ClientEndpoint();
        proofEndpoint.createWebSocketClient("ws://" + cppVerifierUrl);
        //proofEndpoint.latch.await();
        proofEndpoint.webSocketClient.connect();
        proofEndpoint.latch.await();
        proofEndpoint.latch = new CountDownLatch(1);
        proofEndpoint.pos = 0;
        proofEndpoint.webSocketClient.send(claim);
        //proofEndpoint.sendFile(path + "/CD1.data", "CD1.data");
        //proofEndpoint.sendFile(path + "/CD2.data", "CD2.data");
        proofEndpoint.sendFile(path + "/"+claim+"Cloud.data", claim+"Cloud.data");
        proofEndpoint.sendFile(path + "/"+claim+"Cloud.key", claim+"Cloud.key");
        proofEndpoint.webSocketClient.send(String.valueOf(A));
        System.out.println("result");
        proofEndpoint.latch.await();
        int proof[];
        proof = vote_Decrypt(path + "/voteAnswer.data", path + "/" + claim + "SK.key");

        System.out.println("proof array size :"+proof.length);
        for (int k : proof) {
            System.out.println("result :" + k);
        }
        for (int j : proof) {
            String num = String.valueOf(j);
            proofEndpoint.webSocketClient.send(num);
        }

        proofEndpoint.latch.await();
        proofEndpoint.webSocketClient.close();
        // Create and configure the AlertDialog
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Verification");
        builder.setMessage("finish");
        builder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                // User clicked OK, do something if needed
            }
        });
        AlertDialog dialog = builder.create();
        dialog.show();
        //End time
        long end = System.currentTimeMillis();
        long time = end-begin;
        System.out.println();
        System.out.println("Elapsed Time: "+time +" milli seconds");
        Log.d("Result of the multiplication :", String.valueOf(0));

    }
}