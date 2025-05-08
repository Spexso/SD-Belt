package gtu.cse.cse396.sdbelt.shared.model;

public record Response<T>(int status, T result) {
}
 