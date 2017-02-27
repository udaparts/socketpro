
package SPA.ServerSide;

import java.lang.annotation.ElementType;
import java.lang.annotation.Target;

@Target({ElementType.METHOD})
@java.lang.annotation.Retention(java.lang.annotation.RetentionPolicy.RUNTIME)
public @interface RequestAttr {
    public short RequestID();
    public boolean SlowRequest() default false;
}
