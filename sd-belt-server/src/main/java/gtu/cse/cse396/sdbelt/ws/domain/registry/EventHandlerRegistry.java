package gtu.cse.cse396.sdbelt.ws.domain.registry;

import gtu.cse.cse396.sdbelt.ws.domain.model.Event;
import gtu.cse.cse396.sdbelt.ws.domain.model.EventHandler;
import gtu.cse.cse396.sdbelt.ws.domain.model.EventType;

public interface EventHandlerRegistry {
    
    void register(EventType type, Class<? extends Event<?>> dataType, EventHandler<?> handler);

    void remove(EventType type);

    EventHandler<?> getHandler(EventType type);

    Class<? extends Event<?>> getDataType(EventType type);
}
