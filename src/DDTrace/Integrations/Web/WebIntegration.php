<?php

namespace DDTrace\Integrations\Web;

use DDTrace\Integrations\SandboxedIntegration;

class WebIntegration extends SandboxedIntegration
{
    const NAME = 'web';

    /**
     * @var self
     */
    private static $instance;

    /**
     * @return self
     */
    public static function getInstance()
    {
        if (null === self::$instance) {
            self::$instance = new self();
        }
        return self::$instance;
    }

    /**
     * @return string The integration name.
     */
    public function getName()
    {
        return self::NAME;
    }

    public function init()
    {
        // For now we do nothing, as this is done in the bootstrap logic at the moment. We may consider doing this
        // here instead, but leaving this for a future refactoring.
        return SandboxedIntegration::LOADED;
    }

    /**
     * {@inheritdoc}
     */
    public function requiresExplicitTraceAnalyticsEnabling()
    {
        return false;
    }

    /**
     * Loads the generic web request integration.
     *
     * @return int
     */
    public static function load()
    {
        // For now we do nothing, as this is done in the bootstrap logic at the moment. We may consider doing this
        // here instead, but leaving this for a future refactoring.
        return SandboxedIntegration::LOADED;
    }
}
