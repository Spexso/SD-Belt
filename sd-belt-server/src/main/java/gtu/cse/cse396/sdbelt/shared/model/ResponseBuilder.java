package gtu.cse.cse396.sdbelt.shared.model;

import lombok.Builder;

/**
 * Responsible to build new responses for REST requests
 */
@Builder
public class ResponseBuilder<T> {

    private int status;
    private T body;

    public static <T> Response<T> build(int status, T item) {
        return new Response<>(status, item);
    }

}