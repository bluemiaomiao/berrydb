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

public class BaseException extends RuntimeException {
    private static final long serialVersionUID = -6115487863398926192L;
    private DatabaseError currentError = null;
    private String currentInfo = "";

    private static final String exceptionDetailString = "\n Exception Detail: ";

    public BaseException(String errorType, Object... info) {
        this.currentError = new DatabaseError();
        this.currentError.setErrorType(errorType);

        if (info == null) {
            this.currentInfo = "no more exception info";
            return;
        }

        StringBuilder builder = new StringBuilder();
        for (Object o : info) {
            builder.append(o.toString());
            builder.append(" ");
        }

        this.currentInfo = builder.toString();

        try {
            this.currentError.setErrorCode(DatabaseErrorLookup.getErrorCodeByType(errorType));
            this.currentError.setErrorDescription(DatabaseErrorLookup.getErrorDescriptionByType(errorType) +
                    BaseException.exceptionDetailString + currentInfo);
        } catch (Exception e) {
            this.currentError.setErrorCode(0);
            this.currentError.setErrorDescription(DatabaseConstants.UNKNOWN_DESC + BaseException.exceptionDetailString + currentInfo);
        }
    }

    public BaseException(int errorCode, Object... info) {
        this.currentError = new DatabaseError();
        this.currentError.setErrorCode(errorCode);

        if (info == null) {
            this.currentInfo = "no more exception info";
            return;
        }

        StringBuilder builder = new StringBuilder();
        for (Object o : info) {
            builder.append(o.toString());
            builder.append(" ");
        }

        this.currentInfo = builder.toString();

        try {
            this.currentError.setErrorType(DatabaseErrorLookup.getErrorTypeByCode(errorCode));
            this.currentError.setErrorDescription(DatabaseErrorLookup.getErrorDescriptionByCode(errorCode) +
                    BaseException.exceptionDetailString + currentInfo);
        } catch (Exception e) {
            this.currentError.setErrorType(DatabaseConstants.UNKNOWN_DESC);
            this.currentError.setErrorDescription(DatabaseConstants.UNKNOWN_DESC + BaseException.exceptionDetailString + currentInfo);
        }
    }

    @Override
    public String getMessage() {
        return this.currentError.getErrorDescription();
    }

    public String getErrorType() {
        return this.currentError.getErrorType();
    }

    public int getErrorCode() {
        return this.currentError.getErrorCode();
    }
}