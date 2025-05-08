package gtu.cse.cse396.sdbelt.product.infra.controller;

import java.util.List;
import java.util.UUID;

import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.parameters.RequestBody;
import io.swagger.v3.oas.annotations.responses.ApiResponse;
import io.swagger.v3.oas.annotations.responses.ApiResponses;
import lombok.RequiredArgsConstructor;
import gtu.cse.cse396.sdbelt.product.domain.model.Product;
import gtu.cse.cse396.sdbelt.product.domain.service.ProductService;
import gtu.cse.cse396.sdbelt.shared.model.Response;
import gtu.cse.cse396.sdbelt.shared.model.ResponseBuilder;

@RestController
@RequiredArgsConstructor
@RequestMapping("/api/v1")
public class ProductController {
    
    private final ProductService service;

    @Operation(summary = "Get product info", description = "Get product info by id")
    @ApiResponses(value = {
            @ApiResponse(responseCode = "200", description = "Product info retrieved successfully"),
            @ApiResponse(responseCode = "400", description = "Invalid request. Check the provided product ID."),
            @ApiResponse(responseCode = "401", description = "Unauthorized request. Please check your credentials."),
            @ApiResponse(responseCode = "500", description = "Internal server error. Unable to retrieve product info.")
    })
    @GetMapping("/product/{id}")
    public Response<Product> getProduct(@PathVariable UUID id) {
        Product product = service.get(id);
        return ResponseBuilder.build(200, product);
    }

    @Operation(summary = "Get all products", description = "Get all products")
    @ApiResponses(value = {
            @ApiResponse(responseCode = "200", description = "Products retrieved successfully"),
            @ApiResponse(responseCode = "400", description = "Invalid request. Check the provided product ID."),
            @ApiResponse(responseCode = "401", description = "Unauthorized request. Please check your credentials."),
            @ApiResponse(responseCode = "500", description = "Internal server error. Unable to retrieve products.")
    })
    @GetMapping("/products")
    public Response<List<Product>> getAllProducts() {
        List<Product> products = service.list();
        return ResponseBuilder.build(200, products);
    }

    @Operation(summary = "Delete product", description = "Delete product by id")
    @ApiResponses(value = {
            @ApiResponse(responseCode = "200", description = "Product deleted successfully"),
            @ApiResponse(responseCode = "400", description = "Invalid request. Check the provided product ID."),
            @ApiResponse(responseCode = "401", description = "Unauthorized request. Please check your credentials."),
            @ApiResponse(responseCode = "500", description = "Internal server error. Unable to delete product.")
    })
    @DeleteMapping("/product/{id}")
    public Response<UUID> deleteProduct(@PathVariable UUID id) {
        service.delete(id);
        return ResponseBuilder.build(200, id);
    }

    @Operation(summary = "Add product", description = "Add a new product")
    @ApiResponses(value = {
            @ApiResponse(responseCode = "200", description = "Product added successfully"),
            @ApiResponse(responseCode = "400", description = "Invalid request. Check the provided product details."),
            @ApiResponse(responseCode = "401", description = "Unauthorized request. Please check your credentials."),
            @ApiResponse(responseCode = "500", description = "Internal server error. Unable to add product.")
    })
    @PostMapping("/products")
    public Response<Product> addProduct(@RequestBody Product product) {
        service.create(product.name(), product.description());
        return ResponseBuilder.build(200, product);
    }

    @Operation(summary = "Update product", description = "Update product by id")
    @ApiResponses(value = {
            @ApiResponse(responseCode = "200", description = "Product updated successfully"),
            @ApiResponse(responseCode = "400", description = "Invalid request. Check the provided product ID."),
            @ApiResponse(responseCode = "401", description = "Unauthorized request. Please check your credentials."),
            @ApiResponse(responseCode = "500", description = "Internal server error. Unable to update product.")
    })
    @PostMapping("/product/{id}")
    public Response<Product> updateProduct(@PathVariable UUID id, @RequestBody Product product) {
        service.update(id, product.name(), product.description());
        return ResponseBuilder.build(200, product);
    }
}
