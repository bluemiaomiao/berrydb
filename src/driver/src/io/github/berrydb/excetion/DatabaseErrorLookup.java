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

package io.github.berrydb.excetion;

import io.github.berrydb.core.DatabaseConstants;

import java.io.InputStream;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Properties;

public class DatabaseErrorLookup {
    private static HashMap<String, DatabaseError> mapByType;
    private static HashMap<Integer, DatabaseError> mapByCode;

    public DatabaseErrorLookup() throws Exception {
        DatabaseErrorLookup.loadErrorMap();
    }

    @SuppressWarnings("rawTypes")
    private static void loadErrorMap() throws Exception {
        DatabaseErrorLookup.mapByType = new HashMap<>();
        DatabaseErrorLookup.mapByCode = new HashMap<>();

        InputStream streamIn = DatabaseErrorLookup.class.getClassLoader().getResourceAsStream("errors.properties");
        Properties prop = new Properties();
        prop.load(streamIn);

        Enumeration<Object> em = prop.keys();
        while(em.hasMoreElements()) {
            String errorType = (String)em.nextElement();
            String propValue = (String)prop.get(errorType);
            String[] tmp = propValue.split(":");
            String errorCodeStr = tmp[0].trim();
            String errorDescription = tmp[1].trim();

            int errorCode = Integer.parseInt(errorCodeStr);

            DatabaseError error = new DatabaseError();
            error.setErrorCode(errorCode);
            error.setErrorDescription(errorDescription);
            error.setErrorType(errorType);

            DatabaseErrorLookup.mapByCode.put(errorCode, error);
            DatabaseErrorLookup.mapByType.put(errorType, error);
        }
    }

    public static String getErrorDescriptionByType(String type) throws Exception{
        if(DatabaseErrorLookup.mapByType == null) {
            DatabaseErrorLookup.loadErrorMap();
        }

        DatabaseError errorObject = DatabaseErrorLookup.mapByType.get(type);

        if(errorObject == null) {
            return DatabaseConstants.UNKNOWN_DESC;
        }

        return errorObject.getErrorDescription();
    }

    public static String getErrorDescriptionByCode(int code) throws Exception{
        if(DatabaseErrorLookup.mapByCode == null) {
            DatabaseErrorLookup.loadErrorMap();
        }

        DatabaseError errorObject = DatabaseErrorLookup.mapByCode.get(code);

        if(errorObject == null) {
            return DatabaseConstants.UNKNOWN_DESC;
        }

        return errorObject.getErrorDescription();
    }

    public static int getErrorCodeByType(String type) throws Exception {
        if(DatabaseErrorLookup.mapByType == null) {
            DatabaseErrorLookup.loadErrorMap();
        }

        DatabaseError errorObject = DatabaseErrorLookup.mapByType.get(type);

        if(errorObject == null) {
            return DatabaseConstants.UNKNOWN_CODE;
        }

        return errorObject.getErrorCode();
    }

    public static String getErrorTypeByCode(int code) throws Exception {
        if(DatabaseErrorLookup.mapByCode == null) {
            DatabaseErrorLookup.loadErrorMap();
        }

        DatabaseError errorObject = DatabaseErrorLookup.mapByCode.get(code);

        if(errorObject == null) {
            return DatabaseConstants.UNKNOWN_TYPE;
        }

        return errorObject.getErrorType();
    }
}