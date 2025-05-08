package gtu.cse.cse396.sdbelt.scan.infra.controller;

import java.time.LocalDateTime;
import java.util.List;
import java.util.UUID;

import org.springdoc.core.annotations.ParameterObject;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.ModelAttribute;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.parameters.RequestBody;
import io.swagger.v3.oas.annotations.responses.ApiResponse;
import io.swagger.v3.oas.annotations.responses.ApiResponses;
import lombok.RequiredArgsConstructor;
import gtu.cse.cse396.sdbelt.scan.domain.model.GeneralStatistics;
import gtu.cse.cse396.sdbelt.scan.domain.model.ProductStatistics;
import gtu.cse.cse396.sdbelt.scan.domain.model.Scan;
import gtu.cse.cse396.sdbelt.scan.domain.service.ScanService;
import gtu.cse.cse396.sdbelt.scan.infra.model.ScanFilter;
import gtu.cse.cse396.sdbelt.shared.model.Response;
import gtu.cse.cse396.sdbelt.shared.model.ResponseBuilder;

@RestController
@RequiredArgsConstructor
@RequestMapping("/api/v1")
public class ScanController {
    
    private final ScanService service;

    @Operation(summary = "Get all scans", description = "Get all scans")
    @ApiResponses(value = {
            @ApiResponse(responseCode = "200", description = "Scans retrieved successfully"),
            @ApiResponse(responseCode = "400", description = "Invalid request. Check the provided scan ID."),
            @ApiResponse(responseCode = "401", description = "Unauthorized request. Please check your credentials."),
            @ApiResponse(responseCode = "500", description = "Internal server error. Unable to retrieve scans.")
    })
    @GetMapping("/scans")
    public Response<List<Scan>> getAllScans(@ParameterObject @ModelAttribute ScanFilter filter) {
        List<Scan> scans = service.find(filter);
        return ResponseBuilder.build(200, scans);
    }

    @Operation(summary = "Add scan", description = "Add a new scan")
    @ApiResponses(value = {
            @ApiResponse(responseCode = "200", description = "Scan added successfully"),
            @ApiResponse(responseCode = "400", description = "Invalid request. Check the provided scan details."),
            @ApiResponse(responseCode = "401", description = "Unauthorized request. Please check your credentials."),
            @ApiResponse(responseCode = "500", description = "Internal server error. Unable to add scan.")
    })
    @PostMapping("/scans")
    public Response<Scan> addScan(@RequestBody Scan scan) {
        service.create(scan.productId(), scan.isSuccess(), scan.errorMessage());
        return ResponseBuilder.build(200, scan);
    }

    @Operation(summary = "Get general statistics", description = "Get general statistics")
    @ApiResponses(value = {
            @ApiResponse(responseCode = "200", description = "Statistics retrieved successfully"),
            @ApiResponse(responseCode = "401", description = "Unauthorized request. Please check your credentials."),
            @ApiResponse(responseCode = "500", description = "Internal server error. Unable to retrieve statistics.")
    })
    @GetMapping(name = "/scans/statistics", params = {"startDate", "endDate"})
    public Response<GeneralStatistics> getGeneralStatistics(@RequestParam LocalDateTime startDate, @RequestParam LocalDateTime endDate) {
        GeneralStatistics statistics = service.generateStatistics(startDate, endDate);
        return ResponseBuilder.build(200, statistics);
    }

    @Operation(summary = "Get product statistics", description = "Get product statistics")
    @ApiResponses(value = {
            @ApiResponse(responseCode = "200", description = "Statistics retrieved successfully"),
            @ApiResponse(responseCode = "400", description = "Invalid request. Check the provided product ID."),
            @ApiResponse(responseCode = "401", description = "Unauthorized request. Please check your credentials."),
            @ApiResponse(responseCode = "500", description = "Internal server error. Unable to retrieve statistics.")
    })
    @GetMapping(value = "/scans/statistics", params = {"productId", "startDate", "endDate"})
    public Response<ProductStatistics> getProductStatistics(@RequestParam(required = true) UUID productId, @RequestParam LocalDateTime startDate, @RequestParam LocalDateTime endDate) {
        ProductStatistics statistics = service.generateStatistics(productId, startDate, endDate);
        return ResponseBuilder.build(200, statistics);
    }
}
