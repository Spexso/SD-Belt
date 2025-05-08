package gtu.cse.cse396.sdbelt.system.domain.service;

import gtu.cse.cse396.sdbelt.system.domain.model.System;

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
     * @return the current {@link System} object containing system metadata and state
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
     * Updates the system's configuration or metadata, such as its name or description.
     *
     * @param name        the new name for the system
     * @param description the new description for the system
     */
    void update(String name, String description);
}

