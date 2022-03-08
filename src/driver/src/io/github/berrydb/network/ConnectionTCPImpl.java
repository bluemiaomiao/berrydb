/*******************************************************************************
 Copyright (C) 2021 BerryDB Software Inc.
 This application is free software: you can redistribute it and/or modify it
 under the terms of the GNU Affero General Public License, Version 3, as
 published by the Free Software Foundation.

 This application is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR PARTICULAR PURPOSE, See the GNU Affero General Public License for more
 details.

 You should have received a copy of the GNU Affero General Public License along
 with this application. If not, see <http://www.gnu.org/license/>
 *******************************************************************************/

package io.github.berrydb.network;

import io.github.berrydb.excetion.BaseException;
import io.github.berrydb.util.Helper;

import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;

public class ConnectionTCPImpl implements IConnection {

    private Socket clientSocket;
    private InputStream streamIn;
    private OutputStream streamOut;
    private ConfigOptions options;
    private ServerAddress hostAddress;

    public ConnectionTCPImpl(ServerAddress addr, ConfigOptions options) {
        this.hostAddress = addr;
        this.options = options;
    }

    private void connect() throws BaseException {
        if(this.clientSocket != null) {
            return;
        }

        long sleepTime = 100;
        long maxAutoConnectRetryTime = options.getMaxAutoConnectRetryTime();
        long start = System.currentTimeMillis();

        while(true) {
            BaseException lastError = null;
            InetSocketAddress addr = this.hostAddress.getHostAddress();

            try {
                this.clientSocket = new Socket();
                this.clientSocket.connect(addr, this.options.getConnectTimeout());
                this.clientSocket.setTcpNoDelay(false);
                this.clientSocket.setKeepAlive(this.options.getSocketKeepAlive());
                this.clientSocket.setSoTimeout(this.options.getConnectTimeout());
                this.streamIn = new BufferedInputStream(this.clientSocket.getInputStream());
                this.streamOut = this.clientSocket.getOutputStream();
                return;
            } catch (IOException e) {
                e.printStackTrace();
                lastError = new BaseException("DB_NETWORK");
                this.close();
            }

            long executedTime = System.currentTimeMillis() - start;

            if(executedTime >= maxAutoConnectRetryTime) {
                throw lastError;
            }

            if(sleepTime + executedTime > maxAutoConnectRetryTime) {
                sleepTime = maxAutoConnectRetryTime - executedTime;
            }

            try {
                Thread.sleep(sleepTime);
            } catch (InterruptedException e) {
                // pass
            }

            sleepTime = sleepTime * 2;
        }
    }

    @Override
    public void initialize() throws BaseException {
        this.connect();
    }

    @Override
    public void close() {
        if(this.clientSocket != null) {
            try {
                this.clientSocket.close();
            } catch (IOException e) {
                // pass
            } finally {
                this.streamOut = null;
                this.streamIn = null;
                this.clientSocket = null;
            }
        }
    }

    @Override
    public void changeConfigOptions(ConfigOptions options) throws BaseException {
        this.options = options;
        this.close();
        this.connect();
    }

    @Override
    public long sendMessage(byte[] message) throws BaseException {
        long start = System.nanoTime();

        try {
            this.streamOut.write(message);
        } catch (IOException e) {
            throw new BaseException("DB_NETWORK");
        }

        long end = System.nanoTime();
        return Math.abs(end - start);
    }

    @Override
    public byte[] receiveMessage() throws BaseException {

        byte[] buff = new byte[4];

        this.streamIn.mark(4);

        try {
            int runtime = this.streamIn.read(buff);

            if(runtime != 4) {
                this.close();
                throw new BaseException("DB_NETWORK");
            }

            int messageSize = Helper.byteToInt(buff);

            this.streamIn.reset();

            buff = new byte[messageSize];

            runtime = 0;

            int retSize = 0;

            while(runtime < messageSize) {
                retSize = this.streamIn.read(buff, runtime, messageSize - runtime);
                if(retSize == -1) {
                    this.close();
                    throw new BaseException("DB_NETWORK");
                }

                runtime = runtime + retSize;
            }

            if(runtime != messageSize) {
                // StringBuilder builder = new StringBuilder();
                // for(byte b : buff) {
                //     builder.append(String.format("%02x", b));
                // }
                this.close();
                throw new BaseException("DB_INVALID_ARG");
            }

        } catch (IOException e) {
            throw new BaseException("DB_NETWORK");
        }

        return buff;
    }
}