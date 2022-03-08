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

package io.github.berrydb.monitor.exception;

import io.github.berrydb.monitor.core.DatabaseConstants;

public class BaseException extends RuntimeException {
    private static final long serialVersionUID = -6115487863398926195L;

    private DatabaseError error;
    private String infos = "";
    private static final String exceptionStr = "\n Exception Detail:";

    public BaseException(String errorType, Object... info) {
        error = new DatabaseError();
        error.setErrorType(errorType);

        if (info != null) {
            StringBuilder builder = new StringBuilder();
            for (Object obj : info) {
                builder.append(obj.toString());
                builder.append(" ");
            }
            this.infos = builder.toString();
        } else {
            this.infos = "no more exception info";
        }
        try {
            error.setErrorCode(DatabaseErrorLookup.getErrorCodeByType(errorType));
            error.setErrorDescription(DatabaseErrorLookup.getErrorDescriptionByType(errorType) + exceptionStr + infos);
        } catch (Exception e) {
            error.setErrorCode(0);
            error.setErrorDescription(DatabaseConstants.UNKNOWN_DESC + exceptionStr + infos);
        }
    }

    public BaseException(int errorCode, Object... info) {
        error = new DatabaseError();
        error.setErrorCode(errorCode);
        if (info != null) {
            StringBuilder builder = new StringBuilder();
            for (Object obj : info) {
                builder.append(obj.toString());
                builder.append(" ");
            }
            this.infos = builder.toString();
        } else {
            infos = "no more exception info";
        }
        try {
            error.setErrorType(DatabaseErrorLookup.getErrorTypeByCode(errorCode));
            error.setErrorDescription(DatabaseErrorLookup
                    .getErrorDescriptionByCode(errorCode) + exceptionStr + infos);
        } catch (Exception e) {
            error.setErrorType(DatabaseConstants.UNKNOWN_DESC);
            error.setErrorDescription(DatabaseConstants.UNKNOWN_DESC + exceptionStr + infos);
        }
    }

    @Override
    public String getMessage() {
        return error.getErrorDescription();
    }

    public String getErrorType() {
        return error.getErrorType();
    }

    public int getErrorCode() {
        return error.getErrorCode();
    }
}
