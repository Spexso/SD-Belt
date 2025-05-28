package gtu.cse.cse396.sdbelt.ws.domain.model;

public interface Command<T> extends MessageContent {
    CommandType type();

    T data();
}
