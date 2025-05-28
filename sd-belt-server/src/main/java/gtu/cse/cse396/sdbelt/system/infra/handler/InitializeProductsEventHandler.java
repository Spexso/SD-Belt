package gtu.cse.cse396.sdbelt.system.infra.handler;

import java.util.List;

import com.fasterxml.jackson.core.type.TypeReference;
import com.fasterxml.jackson.databind.ObjectMapper;

import gtu.cse.cse396.sdbelt.product.domain.service.ProductService;
import gtu.cse.cse396.sdbelt.system.domain.model.ProductInfo;
import gtu.cse.cse396.sdbelt.ws.domain.model.Event;
import gtu.cse.cse396.sdbelt.ws.domain.model.EventHandler;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;

@RequiredArgsConstructor
@Slf4j
public class InitializeProductsEventHandler implements EventHandler<List<ProductInfo>> {

    private final ProductService productService;
    private final ObjectMapper objectMapper;

    @Override
    public void handle(Event<List<ProductInfo>> event) {
        try {
            List<ProductInfo> products = objectMapper.convertValue(
                    event.data(),
                    new TypeReference<List<ProductInfo>>() {
                    });

            log.info("Received {} products to initialize", products.size());

            for (ProductInfo product : products) {
                productService.create(product.id(), product.name(), product.description(), product.imageUrl());
            }

        } catch (Exception e) {
            log.error("Failed to initialize products", e);
            throw new RuntimeException("Product initialization failed", e);
        }
    }
}
