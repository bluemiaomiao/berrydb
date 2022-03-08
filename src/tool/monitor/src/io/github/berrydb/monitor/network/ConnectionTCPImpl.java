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

package io.github.berrydb.monitor.network;

import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;

import io.github.berrydb.monitor.exception.BaseException;
import io.github.berrydb.monitor.util.Helper;

public class ConnectionTCPImpl implements IConnection{
    private Socket clientSocket;
    private InputStream input  = null;
    private OutputStream output = null;
    private ConfigOptions options;
    private ServerAddress hostAddress;

    public ConnectionTCPImpl(ServerAddress addr, ConfigOptions options) {
        this.hostAddress = addr;
        this.options = options;
    }

    public boolean connect() throws BaseException {
        if (clientSocket != null) {
            return false;
        }

        BaseException lastError = null;
        InetSocketAddress addr = hostAddress.getHostAddress();
        try {
            clientSocket = new Socket();
            clientSocket.connect(addr, options.getConnectTimeout());

            clientSocket.setTcpNoDelay(!options.getUseNagle());
            clientSocket.setKeepAlive(options.getSocketKeepAlive());
            clientSocket.setSoTimeout(options.getSocketTimeout());
            input = new BufferedInputStream(clientSocket.getInputStream());
            output = clientSocket.getOutputStream();
        } catch (IOException ioe) {
            lastError = new BaseException("DB_NETWORK");
            close();
            throw lastError;
        }
        return true;
    }

    public void close() {
        if (clientSocket != null) {
            try {
                clientSocket.close();
            } catch (Exception ignored) {
            } finally {
                input = null;
                output = null;
                clientSocket = null;
            }
        }
    }
    
    @Override
    public void changeConfigOptions(ConfigOptions opts) throws BaseException {
        this.options = opts;
        close();
        connect();
    }

    @Override
    public boolean sendMessage(byte[] msg) {
        try
        {
            if( null == output )
            {
                return false;
            }
            output.write(msg);
            return true;
        }
        catch ( IOException e )
        {
            throw new BaseException ( "DB_NETWORK" ) ;
        }
    }

    @Override
    public byte[] receiveMessage()  {
        byte[] buf = new byte[4];

        input.mark(4);

        try
        {
            int rtn = input.read(buf);

            if (rtn != 4) {
                close();
                throw new BaseException("DB_NETWORK");
            }
            int msgSize = Helper.byteToInt(buf);

            input.reset();

            buf = new byte[msgSize];
            rtn = 0;
            int retSize = 0;
            while (rtn < msgSize) {
                retSize = input.read(buf, rtn, msgSize - rtn);
                if (-1 == retSize) {
                    close();
                    throw new BaseException("DB_NETWORK");
                }
                rtn += retSize;
            }

            if (rtn != msgSize) {
//                StringBuilder builder = new StringBuilder();
//                for (byte by : buf) {
//                    builder.append(String.format("%02x", by));
//                }
                close();
                throw new BaseException("DB_INVALID_ARG");
            }
        }
        catch ( IOException e )
        {
            throw new BaseException ( "DB_NETWORK" ) ;
        }

        return buf;
    }
}
