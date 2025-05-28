package gtu.cse.cse396.sdbelt.ws.infra.registry;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import org.springframework.stereotype.Component;

import com.fasterxml.jackson.databind.ObjectMapper;

import gtu.cse.cse396.sdbelt.product.domain.service.ProductService;
import gtu.cse.cse396.sdbelt.scan.domain.model.ScanEvent;
import gtu.cse.cse396.sdbelt.scan.domain.service.ScanService;
import gtu.cse.cse396.sdbelt.scan.infra.handler.NewScanEventHandler;
import gtu.cse.cse396.sdbelt.scan.infra.repository.JpaScanRepository;
import gtu.cse.cse396.sdbelt.system.domain.model.InitializeProductsEvent;
import gtu.cse.cse396.sdbelt.system.infra.handler.InitializeProductsEventHandler;
import gtu.cse.cse396.sdbelt.ws.domain.model.Event;
import gtu.cse.cse396.sdbelt.ws.domain.model.EventHandler;
import gtu.cse.cse396.sdbelt.ws.domain.model.EventType;
import gtu.cse.cse396.sdbelt.ws.domain.registry.EventHandlerRegistry;

@Component
public class EventHandlerRegistryImpl implements EventHandlerRegistry {

    private final ObjectMapper objectMapper;
    private final ProductService productService;
    private final ScanService scanService;

    private final Map<EventType, EventHandler<?>> handlers;
    private final Map<EventType, Class<? extends Event<?>>> events = new ConcurrentHashMap<>();

    public EventHandlerRegistryImpl(ObjectMapper objectMapper, ScanService scanService,
            ProductService productService) {
        this.objectMapper = objectMapper;
        this.scanService = scanService;
        this.productService = productService;
        this.handlers = new ConcurrentHashMap<>();
        init();
    }

    private void init() {
        register(EventType.NEW_SCAN_EVENT, ScanEvent.class, new NewScanEventHandler(scanService, objectMapper));
        register(EventType.INITIALIZE_EVENT, InitializeProductsEvent.class,
                new InitializeProductsEventHandler(productService, objectMapper));
    }

    @Override
    public void register(EventType type, Class<? extends Event<?>> dataType, EventHandler<?> handler) {
        handlers.put(type, handler);
        events.put(type, dataType);
    }

    @Override
    public EventHandler<?> getHandler(EventType type) {
        return handlers.get(type);
    }

    @Override
    public Class<? extends Event<?>> getDataType(EventType type) {
        return events.get(type);
    }

    @Override
    public void remove(EventType type) {
        handlers.remove(type);
    }
}