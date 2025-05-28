package gtu.cse.cse396.sdbelt.system.domain.model;

public enum BeltDirection {
    FORWARD,
    REVERSE;

    /**
     * Returns the opposite direction of the current direction.
     *
     * @return the opposite direction
     */
    public BeltDirection opposite() {
        return this == FORWARD ? REVERSE : FORWARD;
    }
}
