package com.thomsonreuters.upa.examples.authlock;

import com.thomsonreuters.upa.dacs.*;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

/**
 * 
 * This is the main file for the AuthLockExample application. Its purpose is to
 * demonstrate the functionality of the DACS library.
 * 
 * <p>
 * <em>Running the application:</em>
 * <p>
 * Change directory to the <i>Applications/Examples</i> directory and run <i>ant</i> to
 * build.
 * <p>
 * java -cp ./bin;../../Libs/upaValueAdd.jar;../../Libs/upa.jar;../../Libs/jdacsUpalib.jar
 * com.thomsonreuters.upa.examples.authlock.AuthLockExample
 */
public class AuthLockExample
{
	JDacsLock _dacsInterface = JDacsLock.createJDacsLock();
	DacsError _error = JDacsLock.createDacsError();
	PrimitiveByteBufferPool _pool = new PrimitiveByteBufferPool();
	
	class PrimitiveByteBufferPool
	{
    	// create a primitive ByteBuffer pool
    	private List<ByteBuffer> _smallBuffersPool = new ArrayList<ByteBuffer>();
    	private List<ByteBuffer> _mediumBuffersPool = new ArrayList<ByteBuffer>();
    	private List<ByteBuffer> _largeBuffersPool = new ArrayList<ByteBuffer>();

    	private static final int _smallBufferSize = 10;
    	private static final int _mediumBufferSize = 100;
    	private static final int _largeBufferSize = 1000;
    	
    	// create 6 buffers of each size and add them to pools
    	ByteBuffer buffer;
    	{    		
    		for (int i = 0; i < 6; i++ )
    		{
    			buffer = ByteBuffer.allocate(_smallBufferSize);
    			_smallBuffersPool.add(buffer);
    			buffer = ByteBuffer.allocate(_mediumBufferSize);
    			_mediumBuffersPool.add(buffer);
    			buffer = ByteBuffer.allocate(_largeBufferSize);
    			_largeBuffersPool.add(buffer);
    		}
    	}
    	
    	ByteBuffer getBuffer(int len) {
            if (len <= _smallBufferSize)
            {
            	if (_smallBuffersPool.size() == 0)
            		return null;
            	else
            		return _smallBuffersPool.remove(0);
            }
            else if (len <= _mediumBufferSize)
            {
            	if (_mediumBuffersPool.size() == 0)
            		return null;
            	else
            		return _mediumBuffersPool.remove(0);            	
            }
            else if (len <= _largeBufferSize)
            {
            	if (_largeBuffersPool.size() == 0)
            		return null;
            	else
            		return _largeBuffersPool.remove(0);            	
            }
            else
            	return null;
    	}

    	void bufferToPool(ByteBuffer buffer, int len) {
    		buffer.position(0);
    		buffer.limit(buffer.capacity());
    		
            if (len == _smallBufferSize) {
            	_smallBuffersPool.add(buffer);
            } else if (len == _mediumBufferSize) {
            	_mediumBuffersPool.add(buffer);
            } else if (len == _largeBufferSize) {
            	_largeBuffersPool.add(buffer);
            } else {
            	System.err.println("application error, buffer size not poolable ");
            }
    	}
	}
	
    /**
     * Auth lock.
     */
    public void authLock() {
    	// test calculateLockLength method
    	// set the input parameters: serviceId, operation, productEntityList
    	int serviceId = 5000;
    	char operation = DacsOperations.OR_OPERATION;
        long[] productEntityList = new long[256];
        int productEntityListLength = 1;
        productEntityList[0] = 62;
        
        // calculate the length of the new lock
        System.out.println("calculate lock1 length");
        int len = _dacsInterface.calculateLockLength(serviceId, operation, 
        		productEntityList, productEntityListLength, _error);
        if (len >= 0) {
        	System.out.println("calculateLockLength() Success, length = " + len);
        } else {
        	System.err.println("calculateLockLength() failed " + _error.errorId() + " - " + _error.text());        	
        }

    	// test createLock method
        // create the lock object
        DacsLock lock1 = JDacsLock.createLock();
        
        // use the calculated lock data length to get the ByteBuffer from pool
        ByteBuffer lockData = _pool.getBuffer(len);
        if (lockData == null) {
        	System.err.println("No buffers of this size in pool");        	
        }
        lock1.data(lockData);

        // populate the lock data
        System.out.println("\ncreate lock1 ");
        int ret = _dacsInterface.createLock(serviceId, operation, 
        		productEntityList, productEntityListLength, lock1, _error);
        if (ret == DacsReturnCodes.NO_ERROR) {
            System.out.println("createLock() - Success");
        } else {
        	System.err.println("createLock() failed " + _error.errorId() + " - " + _error.text());
        }

        // create another lock
        productEntityList[1] = 144;	// note that the array must be sorted
        productEntityListLength = 2;
        
        // create the lock object
        DacsLock lock2 = JDacsLock.createLock();
        
        // get the ByteBuffer of arbitrary from pool without checking the lock length first
        lockData = _pool.getBuffer(9);
        lock2.data(lockData);

        // populate the lock data
        System.out.println("\ncreate lock2 ");
        ret = _dacsInterface.createLock(serviceId, operation, 
        		productEntityList, productEntityListLength, lock2, _error);
        if (ret == DacsReturnCodes.NO_ERROR) {
            System.out.println("createLock() - Success");
        } else {
        	System.err.println("createLock() failed " + _error.errorId() + " - " + _error.text());
        }

        // create another lock
        productEntityList[1] = 63;	// note that the array must be sorted
        productEntityList[2] = 144;
        productEntityListLength = 3;
        
        // create the lock object
        DacsLock lock3 = JDacsLock.createLock();
        
        // get the ByteBuffer of arbitrary from pool without checking the lock length first
        lockData = _pool.getBuffer(30);
        lock3.data(lockData);

        // populate the lock data
        System.out.println("\ncreate lock3 ");
        ret = _dacsInterface.createLock(serviceId, operation, 
        		productEntityList, productEntityListLength, lock3, _error);
        if (ret == DacsReturnCodes.NO_ERROR) {
            System.out.println("createLock() - Success");
        } else {
        	System.err.println("createLock() failed " + _error.errorId() + " - " + _error.text());
        }

        // create another lock of a single "0" PE
        productEntityList[0] = 0;
        productEntityList[1] = 0;
        productEntityList[2] = 0;
        productEntityListLength = 1;
        
        // create the lock object
        DacsLock lock4 = JDacsLock.createLock();
        
        // get the ByteBuffer of arbitrary from pool without checking the lock length first
        lockData = _pool.getBuffer(10);
        lock4.data(lockData);

        // populate the lock data
        System.out.println("\ncreate lock4 ");
        ret = _dacsInterface.createLock(serviceId, operation, 
        		productEntityList, productEntityListLength, lock4, _error);
        if (ret == DacsReturnCodes.NO_ERROR){
            System.out.println("createLock() - Success");
        } else {
        	System.err.println("createLock() failed " + _error.errorId() + " - " + _error.text());
        }
        
        // copy lock3 into new lock5
        DacsLock lock5 = JDacsLock.createLock();
        lock5.data(_pool.getBuffer(lock3.length()));
        System.out.println("\ncopy lock3 to lock5 ");
        lock3.copy(lock5, _error);
        if (ret == DacsReturnCodes.NO_ERROR) {
            System.out.println("Lock.Copy() - Success");
        } else {
        	System.err.println("Lock.Copy() failed " + _error.errorId() + " - " + _error.text());
        }

        // compare the locks using method on DacsLock
        System.out.println("\ncompare lock3 and lock5 ");
        boolean equals = lock3.equals(lock5);
        if (equals == true) {
        	System.out.println("locks equal ");
        } else {
        	System.out.println("locks not equal ");
        }
        	
        // compare the locks data using method on JDacsLock
        System.out.println("\ncompare lock3 and lock5 ");
        equals = JDacsLock.equals(lock3.data(), lock5.data());	
        if (equals == true) {
        	System.out.println("locks equal ");
        } else {
        	System.out.println("locks not equal ");
        }
        	
        System.out.println("\nChange Service id in lock2");
        serviceId = 30;
        productEntityList[0] = 62;	// note that the array must be sorted
        productEntityList[1] = 144;
        productEntityListLength = 2;
        
        // reuse lock2, a new lock data will be populated inside the ByteBuffer
        // populate the lock data
        System.out.println("\nreuse lock2 - create ");
        ret = _dacsInterface.createLock(serviceId, operation, 
        		productEntityList, productEntityListLength, lock2, _error);
        if (ret == DacsReturnCodes.NO_ERROR) {
            System.out.println("createLock() - Success");
        } else {
        	System.err.println("createLock() failed " + _error.errorId() + " - " + _error.text());
        }
        
        // Compare is not equal since different PE lists
        System.out.println("\nCompare lock1 and lock2");
        equals = lock1.equals(lock2);
        if (equals == true) {
        	System.out.println("locks equal ");
        } else {
        	System.out.println("locks not equal ");
        }

        // Compare is not equal since different serviceID's and PE's
        System.out.println("\nCompare lock2 and lock3");
        equals = lock2.equals(lock3);
        if (equals == true) {
        	System.out.println("locks equal ");
        } else {
        	System.out.println("locks not equal ");
        }	

        DacsLock [] lockList = new DacsLock[100];
        int lockListLength = 3;
        lockList[0] = lock1;
        lockList[1] = lock2;
        lockList[2] = lock3;
        
        // get the length of combined lock
        System.out.println("\nCalculate combined lock length ");
        len = _dacsInterface.calculateCombineLockLength(lockList, lockListLength, _error);
        if (len >= 0) {
        	System.out.println("calculateCombineLockLength() Success, length = " + len);
        } 
        else 
        {
        	System.err.println("calculateCombineLockLength() failed " + _error.errorId() + " - " + _error.text());        	
        }

        // populate the combined lock
        DacsLock combinedLock = JDacsLock.createLock();
        combinedLock.data(_pool.getBuffer(len));
        
        System.out.println("\nCombine lock1, lock2, and lock3 into combineLock");
        ret = _dacsInterface.combineLock(combinedLock, lockList, lockListLength, _error);
        if (ret == DacsReturnCodes.NO_ERROR) {
        	System.out.println("combineLock() Success");
        } else {
        	System.err.println("combineLock() failed " + _error.errorId() + " - " + _error.text());        	
        }
        
        // return lock1 data to pool 
        int bufSize = lock1.data().capacity();
        _pool.bufferToPool(lock1.data(), bufSize);
    }

    /**
     * The main method.
     *
     * @param args the arguments
     */
    public static void main(String[] args)
    {
    	AuthLockExample dacsExample = new AuthLockExample();
    	dacsExample.authLock();
    	System.exit(0);
    }

}
