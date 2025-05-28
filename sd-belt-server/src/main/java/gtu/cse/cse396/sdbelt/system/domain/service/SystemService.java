package gtu.cse.cse396.sdbelt.system.domain.service;

import java.util.List;

import gtu.cse.cse396.sdbelt.system.domain.model.BeltDirection;
import gtu.cse.cse396.sdbelt.system.domain.model.System;
import gtu.cse.cse396.sdbelt.system.domain.model.SystemLatestInfo;
import gtu.cse.cse396.sdbelt.system.domain.model.SystemStatus;
import gtu.cse.cse396.sdbelt.system.infra.adapter.SystemStatusInfo;

/**
 * Service interface for managing the overall conveyor belt scanning system.
 * <p>
 * Provides lifecycle operations such as start, stop, and restart, as well as
 * configuration updates and status retrieval.
 */
public interface SystemService {

    /**
     * Retrieves the current status and configuration of the scanning system.
     *
     * @return the current {@link System} object containing system metadata and
     *         state
     */
    System get();

    /**
     * Starts the scanning system.
     * <p>
     * This operation typically initializes all components and begins processing
     * products on the conveyor belt.
     */
    void start();

    /**
     * Stops the scanning system.
     * <p>
     * This operation halts all active scanning and processing, effectively putting
     * the system into a paused or offline state.
     */
    void stop();

    /**
     * Restarts the scanning system.
     * <p>
     * Equivalent to calling {@link #stop()} followed by {@link #start()},
     * used to reinitialize system components or recover from errors.
     */
    void restart();

    /**
     * Restarts the scanning system.
     * <p>
     * Equivalent to calling {@link #stop()} followed by {@link #start()},
     * used to reinitialize system components or recover from errors.
     */
    void shutdown();

    /**
     * Updates the system's configuration or metadata, such as its name or
     * description.
     *
     * @param name        the new name for the system
     * @param description the new description for the system
     */
    void update(String name, String description, Integer speed, Double threshold, BeltDirection beltDirection);

    void updateInfo(SystemStatusInfo info);

    void reverse();

    void updateThreshold(Double threshold);

    void updateSpeed(Integer speed);

    void update(SystemStatus status);

    void update(SystemStatus status, Double cpuUsage, Double cpuTemperature, Double memoryUsage);

    void saveStatus(SystemLatestInfo info);

    List<SystemLatestInfo> getLogs();
}
