package com.josecarlos.controlcarbt;


import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;


import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.os.Handler;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;

public class MovimientosBT extends AppCompatActivity {

    final Context context = this;
    Button IdArriba, IdAbajo,IdIzquierda, IdDerecha, IdDesconectar, IdIniciar, IdDetener, IdCargarRutas, IdClear;
    Handler bluetoothIn;
    ListView IdLista;
    final int handlerState = 0;
    private BluetoothAdapter btAdapter = null;
    private BluetoothSocket btSocket = null;
    private StringBuilder DataStringIN = new StringBuilder();
    public ConnectedThread MyConexionBT;
    // Identificador unico de servicio - SPP UUID
    private static final UUID BTMODULEUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
    // String para la direccion MAC
    private static String address = null;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_movimientos_bt);

        IdArriba = (Button) findViewById(R.id.IdArriba);
        IdAbajo = (Button) findViewById(R.id.IdAbajo);
        IdIzquierda = (Button) findViewById(R.id.IdIzquierda);
        IdDerecha = (Button) findViewById(R.id.IdDerecha);
        IdDesconectar = (Button) findViewById(R.id.IdDesconectar);
        IdIniciar = (Button) findViewById(R.id.IdIniciar);
        IdDetener = (Button) findViewById(R.id.IdDetener);
        IdCargarRutas = (Button) findViewById(R.id.IdCargarRutas);
        IdLista = (ListView) findViewById(R.id.IdLista);
        IdClear = (Button) findViewById(R.id.IdClear);

        IdLista.setOnItemClickListener(mDeviceClickListener);

        bluetoothIn = new Handler() {
            public void handleMessage(android.os.Message msg) {
                if (msg.what == handlerState) {
                    String readMessage = (String) msg.obj;
                    DataStringIN.append(readMessage);

                    int endOfLineIndex = DataStringIN.indexOf("#");

                    if (endOfLineIndex > 0) {
                        String dataInPrint = DataStringIN.substring(0, endOfLineIndex);
                        String rutasNombre[] = dataInPrint.split("-");

                        if (rutasNombre.length > 1) {
                            arrayAdapterListView(rutasNombre);
                        }

                        DataStringIN.delete(0, DataStringIN.length());
                    }
                }
            }
        };

        btAdapter = BluetoothAdapter.getDefaultAdapter(); // get Bluetooth adapter
        VerificarEstadoBT();

        IdIniciar.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v)
            { MyConexionBT.write("0");
            }
        });

        IdArriba.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v)
            { MyConexionBT.write("2");
            }
        });
        IdAbajo.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v)
            { MyConexionBT.write("3");
            }
        });
        IdDerecha.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v)
            { MyConexionBT.write("4");
            }
        });
        IdIzquierda.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v)
            { MyConexionBT.write("5");
            }
        });

        IdCargarRutas.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v)
            { MyConexionBT.write("7");
            }
        });

        IdClear.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v)
            {
                IdLista.setAdapter(null);
                MyConexionBT.write("6");
            }
        });




        IdDesconectar.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                MyConexionBT.write("1");
                if (btSocket!=null)
                {
                    try {btSocket.close();
                        Intent i = new Intent(MovimientosBT.this, DispositivosBT.class);
                        startActivity(i);
                    }
                    catch (IOException e)
                    { Toast.makeText(getBaseContext(), "Error", Toast.LENGTH_SHORT).show();;}
                }
                finish();
            }
        });

        IdDetener.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {

                MyConexionBT.write("1");

                AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(
                        context);

                // set title
                alertDialogBuilder.setTitle("Guardar Ruta");
                final TextView input = new TextView (context);
                alertDialogBuilder.setView(input);


                alertDialogBuilder
                        .setMessage("¿Desea Guardar esta ruta?")
                        .setCancelable(false)
                        .setPositiveButton("Si",new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog,int id) {
                              //  input.setText("VER");

                                final AlertDialog.Builder builder = new AlertDialog.Builder(context);
                                LayoutInflater inflater = MovimientosBT.this.getLayoutInflater();
                                final View dialogView = inflater.inflate(R.layout.activity_ruta_dl, null);
                                builder.setView(dialogView);

                                builder.setPositiveButton("Guardar",new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog,int id) {
                                        EditText data = (EditText) dialogView.findViewById(R.id.IdNombreRuta);

                                        MyConexionBT.write(data.getText().toString());


                                    }
                                });
                                AlertDialog alert = builder.create();
                                alert.show();



                            }
                        })
                        .setNegativeButton("No",new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog,int id) {
                                MyConexionBT.write("1");
                                dialog.cancel();
                            }
                        });

                // create alert dialog
                AlertDialog alertDialog = alertDialogBuilder.create();

                // show it
                alertDialog.show();



            }
        });



    }

    // Configura un (on-click) para la lista
    private AdapterView.OnItemClickListener mDeviceClickListener = new AdapterView.OnItemClickListener() {
        public void onItemClick(AdapterView av, View v, int arg2, long arg3) {

           /* // Obtener la dirección MAC del dispositivo, que son los últimos 17 caracteres en la vista
            String info = ((TextView) v).getText().toString();
            String address = info.substring(info.length() - 17);*/


        }
    };


    // This method use an ArrayAdapter to add data in ListView.
    private void arrayAdapterListView(String rut[])
    {
        setTitle("Rutas");

        List<String> dataList = new ArrayList<String>();

        for (int i=0; i< rut.length; i++){
            dataList.add(rut[i]);
        }


        ArrayAdapter<String> arrayAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_list_item_multiple_choice, dataList);
        IdLista.setAdapter(arrayAdapter);

        IdLista.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> adapterView, View view, final int index, long l) {

                Object clickItemObj = adapterView.getAdapter().getItem(index);
                MyConexionBT.write("8");
               // Toast.makeText(MovimientosBT.this, "Selecciono " + clickItemObj.toString(), Toast.LENGTH_SHORT).show();

                AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(
                        context);

                // set title
                alertDialogBuilder.setTitle("Recorrer Ruta");
                final TextView input = new TextView (context);
                alertDialogBuilder.setView(input);

                alertDialogBuilder
                        .setMessage("¿Ruta normal o invertida?")
                        .setCancelable(false)
                        .setPositiveButton("Normal",new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog,int id) {
                                //  input.setText("VER");
                                if (index == 0){
                                    MyConexionBT.write("1x");
                                } else {
                                    MyConexionBT.write("11");
                                }
                                dialog.cancel();


                            }
                        })
                        .setNegativeButton("Invertida",new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog,int id) {
                                if (index == 0){
                                    MyConexionBT.write("0x");
                                } else {
                                    MyConexionBT.write("00");
                                }
                                dialog.cancel();
                            }
                        })
                        .setNeutralButton("Cancelar",new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog,int id) {

                                    MyConexionBT.write("x");

                                dialog.cancel();
                            }
                        });

                // create alert dialog
                AlertDialog alertDialog = alertDialogBuilder.create();

                // show it
                alertDialog.show();
            }
        });
    }

    @Override
    public void onResume()
    {
        super.onResume();
        //Consigue la direccion MAC desde DeviceListActivity via intent
        Intent intent = getIntent();
        //Consigue la direccion MAC desde DeviceListActivity via EXTRA
        address = intent.getStringExtra(DispositivosBT.EXTRA_DEVICE_ADDRESS);//<-<- PARTE A MODIFICAR >->->
        //Setea la direccion MAC
        BluetoothDevice device = btAdapter.getRemoteDevice(address);

        try
        {
            btSocket = createBluetoothSocket(device);
        } catch (IOException e) {
            Toast.makeText(getBaseContext(), "La creacción del Socket fallo", Toast.LENGTH_LONG).show();
        }
        // Establece la conexión con el socket Bluetooth.
        try
        {
            btSocket.connect();
        } catch (IOException e) {
            try {
                btSocket.close();
            } catch (IOException e2) {}
        }
        MyConexionBT = new ConnectedThread(btSocket);
        MyConexionBT.start();
    }

    @Override
    public void onPause()
    {
        super.onPause();
        try
        { // Cuando se sale de la aplicación esta parte permite
            // que no se deje abierto el socket
            btSocket.close();
        } catch (IOException e2) {}
    }

    private BluetoothSocket createBluetoothSocket(BluetoothDevice device) throws IOException
    {
        //crea un conexion de salida segura para el dispositivo
        //usando el servicio UUID
        return device.createRfcommSocketToServiceRecord(BTMODULEUUID);
    }


    //Comprueba que el dispositivo Bluetooth Bluetooth está disponible y solicita que se active si está desactivado
    public void VerificarEstadoBT() {

        if(btAdapter==null) {
            Toast.makeText(getBaseContext(), "El dispositivo no soporta bluetooth", Toast.LENGTH_LONG).show();
        } else {
            if (btAdapter.isEnabled()) {
            } else {
                Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                startActivityForResult(enableBtIntent, 1);
            }
        }
    }



    //Crea la clase que permite crear el evento de conexion
    public class ConnectedThread extends Thread
    {
        private final InputStream mmInStream;
        private final OutputStream mmOutStream;

        public ConnectedThread(BluetoothSocket socket)
        {
            InputStream tmpIn = null;
            OutputStream tmpOut = null;
            try
            {
                tmpIn = socket.getInputStream();
                tmpOut = socket.getOutputStream();
            } catch (IOException e) { }
            mmInStream = tmpIn;
            mmOutStream = tmpOut;
        }

        public void run()
        {
            byte[] buffer = new byte[256];
            int bytes;

            // Se mantiene en modo escucha para determinar el ingreso de datos
            while (true) {
                try {
                    bytes = mmInStream.read(buffer);
                    String readMessage = new String(buffer, 0, bytes);
                    // Envia los datos obtenidos hacia el evento via handler
                    bluetoothIn.obtainMessage(handlerState, bytes, -1, readMessage).sendToTarget();
                } catch (IOException e) {
                    break;
                }
            }
        }
        //Envio de trama
        public void write(String input)
        {
            try {
                mmOutStream.write(input.getBytes());
            }
            catch (IOException e)
            {
                //si no es posible enviar datos se cierra la conexión
                Toast.makeText(getBaseContext(), "La Conexión fallo", Toast.LENGTH_LONG).show();
                finish();
            }
        }
    }
}
