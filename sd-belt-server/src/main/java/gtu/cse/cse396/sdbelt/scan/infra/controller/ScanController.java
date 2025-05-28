package gtu.cse.cse396.sdbelt.scan.infra.controller;

import java.time.LocalDateTime;
import java.time.ZoneOffset;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.springdoc.core.annotations.ParameterObject;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.ModelAttribute;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.responses.ApiResponse;
import io.swagger.v3.oas.annotations.responses.ApiResponses;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import gtu.cse.cse396.sdbelt.scan.domain.model.GeneralStatistics;
import gtu.cse.cse396.sdbelt.scan.domain.model.ProductStatistics;
import gtu.cse.cse396.sdbelt.scan.domain.model.Scan;
import gtu.cse.cse396.sdbelt.scan.domain.model.ScanRequestDTO;
import gtu.cse.cse396.sdbelt.scan.domain.service.ScanService;
import gtu.cse.cse396.sdbelt.scan.infra.model.ScanFilter;
import gtu.cse.cse396.sdbelt.shared.model.Response;
import gtu.cse.cse396.sdbelt.shared.model.ResponseBuilder;
import gtu.cse.cse396.sdbelt.system.domain.service.SystemService;

@RestController
@RequiredArgsConstructor
@RequestMapping("/api/v1")
@Slf4j
public class ScanController {

    private final ScanService service;
    private final SystemService systemService;

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
    public Response<Void> addScan(@RequestBody List<ScanRequestDTO> scans) {
        System.out.println("Getting scans" + scans.size() + " scans: " + scans.toString());
        for (ScanRequestDTO scan : scans) {
            System.out.println("Received scan: " + scan.toString());
        }
        double threshold = 70.0;
        if (systemService.get().threshold() != null) {
            threshold = systemService.get().threshold();
        }
        int numberOfScans = scans.size();
        double score = 0.0;
        Map<String, Double> productConfidenceMap = new HashMap<>();
        for (ScanRequestDTO scan : scans) {
            Double confidence = Double.parseDouble(scan.confidence());
            Double x = Double.parseDouble(scan.x());
            Double y = Double.parseDouble(scan.y());
            String productId = scan.productResult().split("_")[0];
            boolean isSuccess = scan.productResult().split("_")[1].equals("Healthy");
            double health = isSuccess ? confidence : 100.0 - confidence;
            score += health / numberOfScans;
            if (productConfidenceMap.containsKey(productId)) {
                productConfidenceMap.put(productId, productConfidenceMap.get(productId) + confidence);
            } else {
                productConfidenceMap.put(productId, health);
            }
        }
        boolean isSuccess = score >= threshold;
        String productId = "";
        Double productIdScore = 0.0;
        for (Map.Entry<String, Double> entry : productConfidenceMap.entrySet()) {
            if (entry.getValue() > productIdScore) {
                productIdScore = entry.getValue();
                productId = entry.getKey();
            }

        }
        if (isSuccess) {
            service.create(productId, score, isSuccess, null);
        } else {
            service.create(productId, score, isSuccess,
                    "Scan of product" + productId + " with score" + score + "failed");
        }
        return ResponseBuilder.build(200, null);
    }

    @Operation(summary = "Get general statistics", description = "Get general statistics")
    @ApiResponses(value = {
            @ApiResponse(responseCode = "200", description = "Statistics retrieved successfully"),
            @ApiResponse(responseCode = "401", description = "Unauthorized request. Please check your credentials."),
            @ApiResponse(responseCode = "500", description = "Internal server error. Unable to retrieve statistics.")
    })
    @GetMapping(value = "/scans/statistics", params = { "startDate", "endDate" })
    public Response<GeneralStatistics> getGeneralStatistics(@RequestParam LocalDateTime startDate,
            @RequestParam LocalDateTime endDate) {
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
    @GetMapping(value = "/scans/statistics", params = { "productId", "startDate", "endDate" })
    public Response<ProductStatistics> getProductStatistics(@RequestParam(required = true) String productId,
            @RequestParam LocalDateTime startDate, @RequestParam LocalDateTime endDate) {
        ProductStatistics statistics = service.generateStatistics(productId, startDate, endDate);
        return ResponseBuilder.build(200, statistics);
    }
}
