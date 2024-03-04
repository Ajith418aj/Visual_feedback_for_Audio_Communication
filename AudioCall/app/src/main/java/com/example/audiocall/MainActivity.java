package com.example.audiocall;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity implements View.OnClickListener{

    private Button create, join;

    private String button_clicked;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        create = findViewById(R.id.create_btn);
        join = findViewById(R.id.join_btn);

        create.setOnClickListener(this);
        join.setOnClickListener(this);
    }

    @Override
    public void onClick(View view) {
        if(view.getId() == R.id.create_btn) {
            button_clicked = "C"; // Create button clicked
            openCreateActivity(button_clicked);
        } else if (view.getId() == R.id.join_btn) {
            button_clicked = "J"; // Join button clicked
            openCreateActivity(button_clicked);
        }
    }

    public void openCreateActivity(String button_clicked) {
        Intent intent = new Intent(this, CreateRoom.class);
        intent.putExtra("ButtonClicked", button_clicked);
        startActivity(intent);
    }
}